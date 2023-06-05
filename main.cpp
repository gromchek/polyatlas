#include <iostream>
#include <string>
#include <filesystem>
#include <algorithm>
#include "AtlasItem.h"
#include "Atlas.h"
#include "thrid_party/cxxopts.hpp"

namespace fs = std::filesystem;

int substringEntryCount( const std::string &str, const std::string &substr )
{
    if( str.empty() || substr.empty() || substr.length() > str.length() )
    {
        return 0;
    }

    int result = 0;
    std::string::size_type pos = 0;
    while( ( pos = str.find( substr, pos ) ) != std::string::npos )
    {
        ++result;
        pos += substr.length();
    }

    return result;
}

int main( int argc, char *argv[] )
{
    cxxopts::Options options( "polybuild", "Polygon atlas builder" );

    int atlasSize = 2048;
    std::string outputFilename( "atlas" );
    AtlasConfigType configFormat = AtlasConfigType::JSON;
    bool trimAtlasSize = false;
    bool drawDebugLines = false;
    std::string dirPath = ".";
    // clang-format off
    options
    .set_width(80)
    .set_tab_expansion()
    .add_options()
    ("s,max_size", "Atlas max size", cxxopts::value<int>(atlasSize)->default_value("2048"))
    ("o,output", "Output atlas name", cxxopts::value<std::string>(outputFilename)->default_value("atlas"))
    ("c,config", "Output config type: json or lua", cxxopts::value<std::string>()->default_value("json"))
    ("d,dir", "Path to images directory", cxxopts::value<std::string>(dirPath)->default_value("."))

    ("trim", "Trim the atlas size", cxxopts::value<bool>(trimAtlasSize)->default_value("false"))
    ("debug", "Draw debug lines on atlas", cxxopts::value<bool>(drawDebugLines)->default_value("false"))
    ("help", "Print help");
    // clang-format on

    auto result = options.parse( argc, argv );
    if( result.count( "help" ) )
    {
        std::cout << options.help() << std::endl;
        return 0;
    }
    if( result.count( "config" ) )
    {
        auto str = result["config"].as<std::string>();
        std::transform( str.begin(), str.end(), str.begin(), []( unsigned char c ) { return std::tolower( c ); } );
        if( str == "lua" )
        {
            configFormat = AtlasConfigType::LUA;
        }
    }
    if( result.count( "output" ) )
    {
        auto str = result["output"].as<std::string>();
        std::transform( str.begin(), str.end(), str.begin(), []( unsigned char c ) { return std::tolower( c ); } );
        if( !str.empty() )
        {
            outputFilename = str;
        }
    }

    if( !( std::filesystem::exists( dirPath ) && std::filesystem::is_directory( dirPath ) ) )
    {
        std::cout << "Directory is not exist\n";
        return 0;
    }

    std::vector<AtlasItem> items;
    items.reserve( 128 );
    for( const auto &entry : fs::directory_iterator( dirPath ) )
    {
        if( substringEntryCount( entry.path().string(), ".png" ) == 1 )
        {
            items.emplace_back( entry.path().string() );
        }
    }

    if( items.empty() )
    {
        return 0;
    }

    std::sort( items.begin(), items.end(), []( const AtlasItem &item1, const AtlasItem &item2 ) {
        auto size1 = ( item1.GetImageSize().x * item1.GetImageSize().y ) - int( item1.GetArea() );
        auto size2 = ( item2.GetImageSize().x * item2.GetImageSize().y ) - int( item2.GetArea() );
        return size1 > size2;
    } );

    Atlas atlas( atlasSize );
    atlas.SetTrimAtlasSize( trimAtlasSize );
    atlas.SetAtlasName( outputFilename );
    atlas.SetConfigFormat( configFormat );
    atlas.SetDrawDebugLines( drawDebugLines );
    for( const auto &item : items )
    {
        atlas.AddItem( item );
    }

    return 0;
}
