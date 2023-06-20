#include "Misc.h"

using Line = std::pair<cv::Point, cv::Point>;

bool doLineIntersect( const Line &line1, const Line &line2 )
{
    float x1 = line1.first.x;
    float y1 = line1.first.y;
    float x2 = line1.second.x;
    float y2 = line1.second.y;
    float x3 = line2.first.x;
    float y3 = line2.first.y;
    float x4 = line2.second.x;
    float y4 = line2.second.y;

    auto denominator = ( y4 - y3 ) * ( x2 - x1 ) - ( x4 - x3 ) * ( y2 - y1 );
    if( denominator == 0.0f )
    {
        return false;
    }

    auto ua = ( ( x4 - x3 ) * ( y1 - y3 ) - ( y4 - y3 ) * ( x1 - x3 ) ) / denominator;
    auto ub = ( ( x2 - x1 ) * ( y1 - y3 ) - ( y2 - y1 ) * ( x1 - x3 ) ) / denominator;

    if( ua < 0.0f || ua > 1.0f || ub < 0.0f || ub > 1.0f )
    {
        return false;
    }

    return true;
}

bool insidePolygon( const cv::Point &p, const std::vector<cv::Point> &polygon, const int atlasSize )
{
    int count = 0;
    Line ray = { p, { atlasSize, p.y } };
    for( int i = 0; i < polygon.size(); i++ )
    {
        int j = ( i + 1 ) % polygon.size();
        Line edge = { polygon[i], polygon[j] };
        if( doLineIntersect( ray, edge ) )
        {
            count++;
        }
    }
    return count % 2 == 1;
}

bool Math::contoursIntersection( const std::vector<cv::Point> &left, const std::vector<cv::Point> &right )
{
    for( int i = 0; i < left.size(); i++ )
    {
        Line line1 = { left[i], left[( i + 1 ) % left.size()] };
        for( int j = 0; j < right.size(); j++ )
        {
            Line line2 = { right[j], right[( j + 1 ) % right.size()] };
            if( doLineIntersect( line1, line2 ) )
            {
                return true;
            }
        }
    }
    return false;
}

bool Math::contoursInside( const std::vector<cv::Point> &inner, const std::vector<cv::Point> &outer,
                           const int atlasSize )
{
    for( const auto &p : inner )
    {
        if( insidePolygon( p, outer, atlasSize ) && !insidePolygon( p, inner, atlasSize ) )
        {
            return true;
        }
    }
    return false;
}

double perpendicularDistance( const cv::Point &a, const cv::Point &b, const cv::Point &p )
{
    double num = std::abs( ( b.y - a.y ) * p.x - ( b.x - a.x ) * p.y + b.x * a.y - b.y * a.x );
    double den = std::sqrt( ( b.y - a.y ) * ( b.y - a.y ) + ( b.x - a.x ) * ( b.x - a.x ) );
    return num / den;
}

void Math::RDP( const std::vector<cv::Point> &points, const double epsilon, std::vector<cv::Point> &out )
{
    double dmax = 0;
    int index = 0;
    int end = points.size() - 1;
    for( int i = 1; i < end; ++i )
    {
        double d = perpendicularDistance( points[0], points[end], points[i] );
        if( d > dmax )
        {
            index = i;
            dmax = d;
        }
    }

    if( dmax > epsilon )
    {
        std::vector<cv::Point> results1, results2;
        std::vector<cv::Point> first( points.begin(), points.begin() + index + 1 );
        std::vector<cv::Point> last( points.begin() + index, points.end() );
        RDP( first, epsilon, results1 );
        RDP( last, epsilon, results2 );
        out.insert( out.end(), results1.begin(), results1.end() - 1 );
        out.insert( out.end(), results2.begin(), results2.end() );
    }
    else
    {
        out.push_back( points[0] );
        out.push_back( points[end] );
    }
}

std::string convertToUnixFilepath( std::string str )
{
    auto lastindex = str.find_last_of( "." );
    str = str.substr( 0, lastindex );
    std::replace( str.begin(), str.end(), '\\', '/' );

    return str;
}
