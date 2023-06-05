#ifndef ATLAS_H
#define ATLAS_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <map>

enum class AtlasConfigType
{
    JSON,
    LUA
};

class AtlasItem;

class Atlas
{
private:
    const int maxSize;
    struct VirtualAtlas
    {
        const int index;
        const std::vector<AtlasItem> &items;
        const int maxSize;
        cv::Point atlasSize = { 0, 0 };
        std::map<int, cv::Point> itemsPosition;

        VirtualAtlas() = delete;
        VirtualAtlas( const std::vector<AtlasItem> &objects, const int idx, const int dim );

        std::string MakeAtlasName( std::string str ) const;

        bool drop( const AtlasItem &item, const int index );
    };

    std::vector<VirtualAtlas> virtualAtlases;
    std::vector<AtlasItem> items;

    bool drawDebugLines = false;
    bool trimAtlasSize = true;
    std::string atlasName{ "atlas" };
    AtlasConfigType configType = AtlasConfigType::JSON;

    void dropItemsOnAtlas();
    void drawAtlas();

    void writeConfigJson( std::string fileName );
    void writeConfigLua( std::string fileName );

public:
    Atlas() = delete;
    Atlas( const int size );
    ~Atlas();

    void AddItem( const AtlasItem &item );

    void SetDrawDebugLines( bool value );
    void SetTrimAtlasSize( bool value );
    void SetAtlasName( const std::string &str );
    void SetConfigFormat( AtlasConfigType t );
};

inline void Atlas::SetDrawDebugLines( bool value )
{
    drawDebugLines = value;
}

inline void Atlas::SetTrimAtlasSize( bool value )
{
    trimAtlasSize = value;
}

inline void Atlas::SetConfigFormat( AtlasConfigType t )
{
    configType = t;
}

#endif // ATLAS_H
