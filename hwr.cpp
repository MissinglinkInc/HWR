#include "picojson.h"
#include <zinnia.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcgi_stdio.h>

using namespace std;

inline int ishex(int x)
{
    return  (x >= '0' && x <= '9')  ||
        (x >= 'a' && x <= 'f')  ||
        (x >= 'A' && x <= 'F');
}
 
int urldecode(const char *s, char *dec)
{
    char *o;
    const char *end = s + strlen(s);
    int c;
 
    for (o = dec; s <= end; o++) {
        c = *s++;
        if (c == '+') c = ' ';
        else if (c == '%' && (  !ishex(*s++)    ||
                    !ishex(*s++)    ||
                    !sscanf(s - 2, "%2x", &c)))
            return -1;
 
        if (dec) *o = c;
    }
 
    return o - dec;
}
 
int main(void)
{
  zinnia::Recognizer *recognizer = zinnia::Recognizer::create();
  if (!recognizer->open("/usr/local/lib/zinnia/model/tomoe/handwriting-ja.model")) {
    printf("%s\t%d",recognizer->what());
    return -1;
  }
  zinnia::Character *character = zinnia::Character::create();

 
  while(FCGI_Accept() >= 0) {
    character->clear();
    printf("Content-type: application/javascript\r\n\r\n");
  char *query_string;
    query_string = getenv("QUERY_STRING");
    if(query_string == NULL) {
        continue;
    }
    char out[sizeof(query_string)+32];
    if (urldecode(query_string,out) <0) {
        continue;
    }
    picojson::value v;
    std::string err;
    std::string query(out);
  //picojson::parse(v,&out,&out + strlen(out),&err);
  picojson::parse(v,query.begin(),query.end(),&err);
    if (! err.empty()) {
      std::cerr << err << std::endl;
    }
  if (!v.is<picojson::object>()) continue;
        picojson::object& o = v.get<picojson::object>();
  if (!o["canvas_size"].is<picojson::array>()) continue;
  picojson::array canvas_size = o["canvas_size"].get<picojson::array>();
  if (!canvas_size[0].is<double>()) continue;
  if (!canvas_size[1].is<double>()) continue;
  int canvas_size_x = (int)canvas_size[0].get<double>();
  int canvas_size_y = (int)canvas_size[1].get<double>();

  character->set_width(canvas_size_x);
  character->set_height(canvas_size_y);
  
  if (!o["strokes_points"].is<picojson::array>()) continue;
  picojson::array points = o["strokes_points"].get<picojson::array>();
  picojson::array::iterator pit;
  int i = 0;
  for (pit = points.begin(); pit != points.end(); pit++) {
    picojson::array::iterator sit;
    
  if (!pit->is<picojson::array>()) continue;
    picojson::array strokes = pit->get<picojson::array>();
    for (sit = strokes.begin(); sit != strokes.end(); sit++) {
  if (!sit->is<picojson::array>()) continue;
      picojson::array stroke = sit->get<picojson::array>();
  if (!stroke[0].is<double>()) continue;
  if (!stroke[1].is<double>()) continue;
      int x = (int)stroke[0].get<double>();
      int y = (int)stroke[1].get<double>();
      character->add(i,x,y);
    }
    ++i;
  }

  zinnia::Result *result = recognizer->classify(*character, 10);
  if (!result) {
    printf("%s\t%d",recognizer->what());
    return -1;
  }
  /*
  picojson::object jsonp_result;
  picojson::array results;
  for (size_t i = 0; i < result->size(); ++i) {
  picojson::object ro;
    ro["value"] = (picojson::value)result->value(i);
    ro["score"] = (picojson::value)result->score(i);
  results.push_back((picojson::value)ro);
  }
  jsonp_result["result"] = (picojson::value)results;
  picojson::value v1(jsonp_result);
    printf("_mlHWRCallback(%s);",v1.serialize().c_str());
    */

  printf("_mlHWRCallback({\"result\":[");
  size_t length = result->size();
  for (size_t i = 0; i < length; ++i) {
    printf("{\"value\":\"%s\",\"score\":%f}",result->value(i),result->score(i));
    if (i-1 == length);
    else
      printf(",");
  }
  printf("]});");


  delete result;
  }

  delete character;
  delete recognizer;
return 1;
}

