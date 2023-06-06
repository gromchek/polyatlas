#include "Atlas.h"
#include "AtlasItem.h"
#include <fstream>
#include "thrid_party/json.hpp"
#include "Misc.h"

bool doRectIntersect( const cv::Rect &left, const cv::Rect &right )
{
    return ( left & right ).area() > 0;
}
unsigned long upper_power_of_two( unsigned long v )
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

namespace nlohmann
{
template <>
struct adl_serializer<cv::Point>
{
    static void to_json( json &j, const cv::Point &p )
    {
        j = { p.x, p.y };
    }

    static void from_json( const json &j, cv::Point &p )
    {
        p.x = j[0];
        p.y = j[1];
    }
};
} // namespace nlohmann

void Atlas::dropItemsOnAtlas()
{
    if( items.empty() )
    {
        return;
    }

    std::vector<bool> addedItem( items.size(), false );

    while( 1 )
    {
        virtualAtlases.push_back( { items, int( virtualAtlases.size() ), maxSize } );

        for( auto i = 0; i < addedItem.size(); i++ )
        {
            if( addedItem[i] )
            {
                continue;
            }
            addedItem[i] = virtualAtlases.back().drop( items[i], i );
        }

        if( std::find( addedItem.begin(), addedItem.end(), false ) == addedItem.end() )
        {
            break;
        }
    }
}

void Atlas::drawAtlas()
{
    if( virtualAtlases.empty() )
    {
        return;
    }

    for( const auto &entry : virtualAtlases )
    {
        cv::Size size( maxSize, maxSize );

        if( trimAtlasSize )
        {
            size.width = upper_power_of_two( entry.atlasSize.x );
            size.height = upper_power_of_two( entry.atlasSize.y );
        }

        cv::Mat atlas = cv::Mat( size, CV_8UC4, cv::Scalar( 0, 0, 0, 0 ) );

        for( const auto &[k, v] : entry.itemsPosition )
        {
            cv::Mat crop( items[k].GetCropImage() );
            cv::Rect roi( v, crop.size() );
            crop.copyTo( cv::Mat( atlas, roi ), items[k].GetCropMask() );
        }
        cv::imwrite( entry.MakeAtlasName( atlasName ) + ".png", atlas );
    }

    std::string configFilename;
    configFilename.append( atlasName );
    if( configType == AtlasConfigType::JSON )
    {
        writeConfigJson( configFilename );
    }
    else
    {
        writeConfigLua( configFilename );
    }

    if( drawDebugLines )
    {
        for( const auto &entry : virtualAtlases )
        {
            cv::Size size( maxSize, maxSize );

            if( trimAtlasSize )
            {
                size.width = upper_power_of_two( entry.atlasSize.x );
                size.height = upper_power_of_two( entry.atlasSize.y );
            }

            cv::Mat atlas = cv::Mat( size, CV_8UC4, cv::Scalar( 0, 0, 0, 0 ) );

            for( const auto &[k, v] : entry.itemsPosition )
            {
                cv::Mat crop( items[k].GetCropImage() );
                cv::Rect roi( v, crop.size() );
                crop.copyTo( cv::Mat( atlas, roi ), items[k].GetCropMask() );

                const auto &contour = items[k].GetContours( v );
                const auto &indices = items[k].GetIndices();
                for( int i = 0; i < indices.size(); i += 3 )
                {
                    cv::Point pt1( contour[indices[i]].x, contour[indices[i]].y );
                    cv::Point pt2( contour[indices[i + 1]].x, contour[indices[i + 1]].y );
                    cv::Point pt3( contour[indices[i + 2]].x, contour[indices[i + 2]].y );
                    cv::line( atlas, pt1, pt2, cv::Scalar( 0, 0, 255, 255 ), 1 );
                    cv::line( atlas, pt2, pt3, cv::Scalar( 0, 0, 255, 255 ), 1 );
                    cv::line( atlas, pt3, pt1, cv::Scalar( 0, 0, 255, 255 ), 1 );
                }
            }
            cv::imwrite( entry.MakeAtlasName( atlasName ) + ".debug.png", atlas );
        }
    }
}

void Atlas::writeConfigJson( std::string fileName )
{
    nlohmann::json object;

    for( const auto &entry : virtualAtlases )
    {
        for( const auto &[k, v] : entry.itemsPosition )
        {
            nlohmann::json atlasEntry;
            atlasEntry[items[k].GetFilepath()] = { { "atlas", atlasName + std::to_string( entry.index ) },
                                                   { "indices", items[k].GetIndices() },
                                                   { "vertexPosition", items[k].GetContours( v ) } };
            object.push_back( atlasEntry );
        }
    }

    fileName.append( ".json" );
    std::ofstream file( fileName );
    file << object.dump();
}

void Atlas::writeConfigLua( std::string fileName )
{
    fileName.append( ".lua" );
    std::ofstream file( fileName );

    file << atlasName << " = {";
    for( const auto &entry : virtualAtlases )
    {
        for( const auto &[k, v] : entry.itemsPosition )
        {
            file << "['" << items[k].GetFilepath() << "'] = {";

            file << "['atlas'] = '" << atlasName + std::to_string( entry.index ) << "',";

            file << "['indices'] = {";
            const auto &indices = items[k].GetIndices();
            for( int i = 0; i < indices.size() - 1; i++ )
            {
                file << indices[i] << ",";
            }
            file << indices[indices.size() - 1];
            file << "},";

            file << "['vertexPosition'] = {";
            const auto &contours = items[k].GetContours( v );
            for( int i = 0; i < contours.size() - 1; i++ )
            {
                file << "{" << contours[i].x << "," << contours[i].y << "},";
            }
            file << "{" << contours[contours.size() - 1].x << "," << contours[contours.size() - 1].y << "}";
            file << "}";

            file << "},";
        }
    }

    file << "}";
}

Atlas::Atlas( const int size ) : maxSize( upper_power_of_two( size ) )
{
    items.reserve( 128 );
}

Atlas::~Atlas()
{
    dropItemsOnAtlas();
    drawAtlas();
}

void Atlas::AddItem( const AtlasItem &item )
{
    const auto &size = item.GetImageSize();
    if( size.x >= maxSize || size.y >= maxSize || item.GetArea() == 0.0f )
    {
        return;
    }

    items.push_back( item );
}

void Atlas::SetAtlasName( const std::string &str )
{
    atlasName = str;
}

Atlas::VirtualAtlas::VirtualAtlas( const std::vector<AtlasItem> &objects, const int idx, const int dim ) :
    index( idx ), items( objects ), maxSize( dim )
{
}

std::string Atlas::VirtualAtlas::MakeAtlasName( std::string str ) const
{
    return str.append( std::to_string( index ) );
}

bool Atlas::VirtualAtlas::drop( const AtlasItem &item, const int index )
{
    cv::Point p( 0, 0 );
    if( itemsPosition.empty() )
    {
        const auto &size = item.GetImageSize();
        if( size.x + p.x <= maxSize && size.y + p.y <= maxSize )
        {
            itemsPosition[index] = p;
            atlasSize.x = std::max( atlasSize.x, p.x + item.GetImageSize().x );
            atlasSize.y = std::max( atlasSize.y, p.y + item.GetImageSize().y );
            return true;
        }
    }

    const auto &size = item.GetImageSize();
    while( 1 )
    {
        bool intersect = false;
        for( const auto &[k, v] : itemsPosition )
        {
            const auto &itemSize = items[k].GetImageSize();
            if( doRectIntersect( { p.x, p.y, size.x, size.y }, { v.x, v.y, itemSize.x, itemSize.y } ) )
            {
                const auto &itemContours = items[k].GetContours( v );
                const auto &contours = item.GetContours( p );
                if( Math::contoursIntersection( itemContours, contours ) )
                {
                    intersect = true;
                    break;
                }
                if( Math::contoursInside( contours, itemContours, maxSize ) )
                {
                    intersect = true;
                    break;
                }
            }
        }

        if( !intersect )
        {
            itemsPosition[index] = p;
            break;
        }

        auto newPoint = p;
        auto dX = int( float( maxSize ) * 0.005f ) + 1;
        auto dY = int( float( maxSize ) * 0.005f ) + 1;
        newPoint.x += dX;
        int step = 0;
        if( newPoint.x + size.x >= maxSize )
        {
            newPoint.x = 0;
            newPoint.y += dY;
            step++;
        }
        if( newPoint.y + size.y >= maxSize )
        {
            newPoint.y = 0;
            step++;
        }
        if( step > 1 )
        {
            return false;
        }
        p = newPoint;
    }

    atlasSize.x = std::max( atlasSize.x, p.x + item.GetImageSize().x );
    atlasSize.y = std::max( atlasSize.y, p.y + item.GetImageSize().y );

    return true;
}
