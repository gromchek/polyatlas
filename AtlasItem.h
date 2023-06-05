#ifndef ATLASITEM_H
#define ATLASITEM_H

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

class AtlasItem final
{
private:
    std::string filePath;
    std::vector<std::vector<cv::Point>> approxContours;
    std::vector<unsigned int> indices;
    cv::Rect boundRect;
    float area = 0.0f;

    cv::Mat unchangedImg;
    cv::Mat cropImage;
    cv::Mat cropMask;
    void calcContours();
    void loadImage();

public:
    AtlasItem() = delete;
    AtlasItem( std::string &&file );

    const cv::Mat &GetCropImage() const;
    const cv::Mat &GetCropMask() const;

    const std::string &GetFilepath() const;
    std::vector<cv::Point> GetContours( const cv::Point &offset = { 0, 0 } ) const;
    cv::Point GetImageSize() const;
    const std::vector<unsigned int> &GetIndices() const;

    std::vector<cv::Point2f> GetTextureCoords( const cv::Point &vertOffset, const cv::Point2f &texSize ) const;

    float GetArea() const;
};

#endif // ATLASITEM_H
