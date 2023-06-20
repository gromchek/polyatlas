#ifndef MISC_H
#define MISC_H
#include <opencv2/core/types.hpp>

namespace Math
{
bool contoursIntersection( const std::vector<cv::Point> &left, const std::vector<cv::Point> &right );
bool contoursInside( const std::vector<cv::Point> &inner, const std::vector<cv::Point> &outer, const int atlasSize );
void RDP( const std::vector<cv::Point> &points, const double epsilon, std::vector<cv::Point> &out );
} // namespace Math

std::string convertToUnixFilepath( std::string str );

#endif // MISC_H
