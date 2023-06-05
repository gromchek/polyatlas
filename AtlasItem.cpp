#include "AtlasItem.h"
#include "thrid_party/earcut.hpp"
#include "Misc.h"

namespace mapbox
{
namespace util
{

template <>
struct nth<0, cv::Point>
{
    inline static auto get( const cv::Point &t )
    {
        return t.x;
    };
};
template <>
struct nth<1, cv::Point>
{
    inline static auto get( const cv::Point &t )
    {
        return t.y;
    };
};

} // namespace util
} // namespace mapbox

void AtlasItem::calcContours()
{
    cv::Mat grayScale;
    int iterations = 4;
    cv::dilate( unchangedImg, grayScale, cv::getStructuringElement( cv::MORPH_RECT, { 5, 5 } ), { -1, -1 },
                iterations );
    cv::cvtColor( grayScale, grayScale, cv::COLOR_RGBA2GRAY );
    cv::Mat binary;
    cv::threshold( grayScale, binary, 0, 255, cv::THRESH_BINARY );

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours( binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE );

    if( contours.empty() )
    {
        return;
    }

    if( contours.size() > 1 )
    {
        for( auto i = 1; i < contours.size(); i++ )
        {
            contours[0].insert( contours[0].end(), contours[i].begin(), contours[i].end() );
        }

        std::vector<cv::Point> convexHull;
        cv::convexHull( cv::Mat( contours[0] ), convexHull );

        contours[0].clear();
        contours[0].reserve( convexHull.size() );
        contours[0].insert( contours[0].end(), convexHull.begin(), convexHull.end() );
    }

    approxContours.resize( 1 );
    Math::RDP( contours[0], 5.0, approxContours[0] );
    boundRect = cv::boundingRect( approxContours[0] );
    for( auto &c : approxContours[0] )
    {
        c -= cv::Point{ boundRect.x, boundRect.y };
    }
    cropImage = unchangedImg( boundRect ).clone();
    cropMask = binary( boundRect ).clone();

    const int n = approxContours[0].size();
    for( int i = 0; i < n; i++ )
    {
        int j = ( i + 1 ) % n;
        area += approxContours[0][i].x * approxContours[0][j].y - approxContours[0][i].y * approxContours[0][j].x;
    }
    area = std::fabs( area * 0.5f );

    indices = std::move( mapbox::earcut<unsigned int>( std::vector<std::vector<cv::Point>>{ approxContours[0] } ) );
}

void AtlasItem::loadImage()
{
    unchangedImg = cv::imread( filePath, cv::IMREAD_UNCHANGED );

    if( !unchangedImg.empty() )
    {
        calcContours();
    }
}

AtlasItem::AtlasItem( std::string &&file ) : filePath( std::move( file ) )
{
    loadImage();

    auto lastindex = filePath.find_last_of( "." );
    filePath = filePath.substr( 0, lastindex );
    std::replace( filePath.begin(), filePath.end(), '\\', '/' );
}

const cv::Mat &AtlasItem::GetCropImage() const
{
    return cropImage;
}

const cv::Mat &AtlasItem::GetCropMask() const
{
    return cropMask;
}

const std::string &AtlasItem::GetFilepath() const
{
    return filePath;
}

std::vector<cv::Point> AtlasItem::GetContours( const cv::Point &offset ) const
{
    if( offset.x == 0 && offset.y == 0 )
    {
        return approxContours[0];
    }

    std::vector<cv::Point> result;
    result.reserve( approxContours[0].size() );
    for( const auto &c : approxContours[0] )
    {
        result.emplace_back( c + offset );
    }

    return result;
}

cv::Point AtlasItem::GetImageSize() const
{
    return { boundRect.width, boundRect.height };
}

const std::vector<unsigned int> &AtlasItem::GetIndices() const
{
    return indices;
}

std::vector<cv::Point2f> AtlasItem::GetTextureCoords( const cv::Point &vertOffset, const cv::Point2f &texSize ) const
{
    std::vector<cv::Point2f> texCoords;

    const auto &contour = GetContours( vertOffset );
    texCoords.reserve( indices.size() );
    for( int i = 0; i < indices.size(); i += 3 )
    {
        cv::Point2f t0( contour[indices[i]].x / texSize.x, 1.0f - contour[indices[i]].y / texSize.y );
        cv::Point2f t1( contour[indices[i + 1]].x / texSize.x, 1.0f - contour[indices[i + 1]].y / texSize.y );
        cv::Point2f t2( contour[indices[i + 2]].x / texSize.x, 1.0f - contour[indices[i + 2]].y / texSize.y );
        texCoords.push_back( t0 );
        texCoords.push_back( t1 );
        texCoords.push_back( t2 );
    }

    return texCoords;
}

float AtlasItem::GetArea() const
{
    return area;
}
