#include <iiColorManager/ColorValue.h>
#include <type_traits>
int main(){
    static_assert(std::is_standard_layout_v<iiColorManager::Rgba> && std::is_aggregate_v<iiColorManager::Rgba>);
    auto value=iiColorManager::parseHexValue("#ff000080");
    return !value || *value!=iiColorManager::Rgba{1,0,0,128/255.} || !iiColorManager::validRgba(*value);
}
