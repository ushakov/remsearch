#ifndef _PLACEMARK_STORAGE_H_
#define _PLACEMARK_STORAGE_H_

#include <map>
#include <string>

class PlacemarkStorage
{
public:
    void AddFromFile(std::string name);
    
    std::map<int, std::string> names;
    std::map<int, float> lat;
    std::map<int, float> lng;
};

#endif /* _PLACEMARK_STORAGE_H_ */
