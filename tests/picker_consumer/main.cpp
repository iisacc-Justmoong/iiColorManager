#include <iiColorManager/ColorManager.h>
int main(){
    iiColorManager::Picker picker;
    auto color=iiColorManager::parseHex("#ff000080");
    return !color || !picker.begin(*color) || picker.color()!=*color || iiColorManager::modes().size()!=9;
}
