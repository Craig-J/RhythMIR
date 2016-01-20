#pragma once
//	Source: https://github.com/catnapgames/TestNLTmxMap
//	Edited by Craig Jeffrey

#include <string>
#include <vector>
#include <memory>

using std::string;
using std::vector;
using std::unique_ptr;

class NLTmxMapTile {
public:
	string id;
	int width;
	int height;
	string filename;
};

class NLTmxMapTileset {
public:
	int firstGid;
	string name;
	int tileWidth;
	int tileHeight;
	vector<unique_ptr<NLTmxMapTile>> tiles;
};

class NLTmxMapLayer {
public:
    string name;
    int width;
    int height;
    vector<int> data;
};

struct NLTmxMapObjectProperty {
    string name;
    string value;
};

class NLTmxMapObject {
public:
    string name;
    int gid;
    int x;
    int y;
    int width;
    int height;
	float rotation;
	vector<unique_ptr<NLTmxMapObjectProperty>> properties;
};

class NLTmxMapObjectGroup {
public:
    string name;
    int width;
    int height;
    bool visible;
    
	vector<unique_ptr<NLTmxMapObject>> objects;
};

class NLTmxMap {
public:
    
    int width;
    int height;
    int tileWidth;
    int tileHeight;
	int totalTileCount;
    
	vector<unique_ptr<NLTmxMapTileset>> tilesets;
	vector<unique_ptr<NLTmxMapLayer>> layers;
	vector<unique_ptr<NLTmxMapObjectGroup>> groups;
};

NLTmxMap* NLLoadTmxMap(const char *xml );