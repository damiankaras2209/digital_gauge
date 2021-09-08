//#include "settingsIndex.h"
//#include <string>
//
////Settings* settings = Settings::getInstance();
//
//
//
////String getSettingsIndex() {
////	String ss;
////	ss =
////		"<meta charset='UTF-8'>"
////		"<script>"
////		"function ajaxpost(){"
////		"  var data = new FormData();"
////		"  data.append('offsetX', document.getElementById('offsetX').value);"
////		"  data.append('offsetY', document.getElementById('offsetY').value);"
////		"  data.append('ellipseA', document.getElementById('ellipseA').value);"
////		"  data.append('ellipseB', document.getElementById('ellipseB').value);"
////		"  data.append('needleCenterRadius', document.getElementById('needleCenterRadius').value);"
////		"  data.append('needleCenterOffset', document.getElementById('needleCenterOffset').value);"
////		"  data.append('needleLength', document.getElementById('needleLength').value);"
////		"  data.append('needleBottomWidth', document.getElementById('needleBottomWidth').value);"
////		"  data.append('needleTopWidth', document.getElementById('needleTopWidth').value);"
////		"  var xhr = new XMLHttpRequest();"
////		"  xhr.open('POST', 'settingsSet');"
////		"  xhr.onload = function(){ console.log(this.response); };"
////		"  xhr.send(data);"
////		"  return false;}"
////		"</script>"
////		"<form onsubmit='return ajaxpost()'>"
////		"<h1>Ustawienia</h1>"
////		"<p>Przesunięcie X</p><input onchange='ajaxpost()' value='"+String(settings->offsetX)+"' type='number' id='offsetX'>"
////		"<p>Przesunięcie Y</p><input onchange='ajaxpost()' value='"+String(settings->offsetY)+"' type='number' id='offsetY'>"
////		"<p>Szerokość elipsy</p><input onchange='ajaxpost()' value='"+String(settings->ellipseA)+"' type='number' id='ellipseA'>"
////		"<p>Wysokość elipsy</p><input onchange='ajaxpost()' value='"+String(settings->ellipseB)+"' type='number' id='ellipseB'>"
////		"<p>Promień środka wskazówki</p><input onchange='ajaxpost()' value='"+String(settings->needleCenterRadius)+"' type='number' id='needleCenterRadius'>"
////		"<p>Odległość środka wskazówki od środka elipsy</p><input onchange='ajaxpost()' value='"+String(settings->needleCenterOffset)+"' type='number' id='needleCenterOffset'>"
////		"<p>Długość wskazówki</p><input onchange='ajaxpost()' value='"+String(settings->needleLength)+"' type='number' id='needleLength'>"
////		"<p>Szerokość podstawy wskazówki</p><input onchange='ajaxpost()' value='"+String(settings->needleBottomWidth)+"' type='number' id='needleBottomWidth'>"
////		"<p>Szerokość końcówki wskazówki</p><input onchange='ajaxpost()' value='"+String(settings->needleTopWidth)+"' type='number' id='needleTopWidth'>"
////	// 	"<p>Kolor</p><input type='color' id='color'>"
////	 	"</br><input type='submit'>"
////		"</form>";
////	return ss;
////}
//
//uint32_t color24to16(uint32_t color888)
//{
//  uint16_t r = (color888 >> 8) & 0xF800;
//  uint16_t g = (color888 >> 5) & 0x07E0;
//  uint16_t b = (color888 >> 3) & 0x001F;
//
//  return (r | g | b);
//}
//
//unsigned int parseColor(const std::string& str) {
//    unsigned int ret;
//    std::stringstream ss;
//    ss << std::hex << str.substr(1);
//    ss >> ret;
//    return color24to16(ret);
//}
//
//void setSettings() {
//
//	Screen* screen = Screen::getInstance();
//
//
//
////	settings->offsetX = atoi(server.arg("offsetX").c_str());
////	settings->offsetY = atoi(server.arg("offsetY").c_str());
////	settings->ellipseA = atoi(server.arg("ellipseA").c_str());
////	settings->ellipseB = atoi(server.arg("ellipseB").c_str());
////	settings->needleCenterRadius = atoi(server.arg("needleCenterRadius").c_str());
////	settings->needleCenterOffset = atoi(server.arg("needleCenterOffset").c_str());
////	settings->needleLength = atoi(server.arg("needleLength").c_str());
////	settings->needleBottomWidth = atoi(server.arg("needleBottomWidth").c_str());
////	settings->needleTopWidth = atoi(server.arg("needleTopWidth").c_str());
//
////	settings->save();
//
//	//settings->needleCenterColor = parseColor(server.arg("color").c_str());
//	screen->reset();
//}