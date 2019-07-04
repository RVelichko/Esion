#include <iostream>

#include <curl/curl.h>

#include "Log.hpp"
#include "GeoRequester.hpp"


using namespace server;

static size_t RecvFn(char* data, size_t size, size_t nmemb, void* buf) {
    reinterpret_cast<std::string*>(buf)->append(data, size * nmemb);
    return size * nmemb;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


GeoRequester::GeoRequester(const std::string& api_key)
    : _api_key(api_key) {
    LOG(DEBUG) << "API key: " << _api_key;
}


GeoRequester::~GeoRequester() {
    LOG(DEBUG);
}


Json GeoRequester::request(const std::string& coll) {
    Json jgeo = {
        {"addr_line", coll},
        {"point", {0.0, 0.0}}
    };
    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_URL, "https://geocode-maps.yandex.ru/1.x/");
        curl_easy_setopt(curl, CURLOPT_USERAGENT,
                         "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) "\
                         "Chrome/75.0.3770.100 Safari/537.36");
        struct curl_slist *hlist = NULL;
        hlist = curl_slist_append(hlist, "Accept: */*");
        hlist = curl_slist_append(hlist, "Pragma: no-cache");
        hlist = curl_slist_append(hlist, "Cache-Control: no-cache");
        hlist = curl_slist_append(hlist, "Accept-Language: ru-RU,ru;q=0.8,en-US;q=0.6,en;q=0.4");
        hlist = curl_slist_append(hlist, "Accept-Encoding: identity");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hlist);
        std::string postfields = "sco=longlat&lang=ru_RU&format=json&apikey=" + _api_key + "&geocode=" + coll;
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postfields.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,  postfields.c_str());
        std::string body_buf;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body_buf);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RecvFn);
        CURLcode res = curl_easy_perform(curl);
        if (res not_eq CURLE_OK) {
            LOG(ERROR) << curl_easy_strerror(res);
        } else {
            //LOG(DEBUG) << "HEADER[" << header_buf.size() << "]: \n" << header_buf.c_str() << "\n";
            LOG(DEBUG) << "BODY[" << body_buf.size() << "]: \n" << body_buf.c_str() << "\n";
            try {
                auto jres = Json::parse(body_buf);
                auto jresp = jres.find("response");
                if (jresp not_eq jres.end() and jresp->is_object()) {
                    auto jgocol = jresp->find("GeoObjectCollection");
                    if (jgocol not_eq jresp->end() and jgocol->is_object()) {
                        auto jfmems = jgocol->find("featureMember");
                        if (jfmems not_eq jgocol->end() and jfmems->is_array()) {
                            auto jfmem = (*jfmems)[0];
                            auto jgobj = jfmem.find("GeoObject");
                            if (jgobj not_eq jfmem.end() and jgobj->is_object()) {
                                auto jmdproperty =  jgobj->find("metaDataProperty");
                                if (jmdproperty not_eq jgobj->end() and jmdproperty->is_object()) {
                                    auto jgcmdata = jmdproperty->find("GeocoderMetaData");
                                    if (jgcmdata not_eq jmdproperty->end() and jgcmdata->is_object()) {
                                        auto jtext = jgcmdata->find("text");
                                        if (jtext not_eq jgcmdata->end() and jtext->is_string()) {
                                            jgeo["addr_line"] = *jtext;
                                        } else {
                                            LOG(ERROR) << "Can`t find response.GeoObjectCollection.featureMember[0].GeoObject."
                                                       << "metaDataProperty.GeocoderMetaData.text!";
                                        }
                                    } else {
                                        LOG(ERROR) << "Can`t find response.GeoObjectCollection.featureMember[0].GeoObject."
                                                   << "metaDataProperty.GeocoderMetaData!";
                                    }
                                }
                                auto jpoint = jgobj->find("Point");
                                if (jpoint not_eq jgobj->end() and jpoint->is_object()) {
                                    auto jpos = jpoint->find("pos");
                                    if (jpos not_eq jpoint->end() and jpos->is_string()) {
                                        std::string pos_str = *jpos;
                                        size_t p = pos_str.find(' ');
                                        if (p not_eq std::string::npos and p < pos_str.size()) {
                                            std::string lo_str = pos_str.substr(0, p);
                                            std::string la_str = pos_str.substr(p + 1);
                                            jgeo["point"][0] = std::atof(la_str.c_str());
                                            jgeo["point"][1] = std::atof(lo_str.c_str());
                                        }
                                    } else {
                                        LOG(ERROR) << "Can`t find response.GeoObjectCollection.featureMember[0]."
                                                   << "GeoObject.Point.pos!";
                                    }
                                } else {
                                    LOG(ERROR) << "Can`t find response.GeoObjectCollection.featureMember[0].GeoObject.Point!";
                                }
                            } else {
                                LOG(ERROR) << "Can`t find response.GeoObjectCollection.featureMember[0].GeoObject."
                                           << "metaDataProperty!";
                            }
                        } else {
                            LOG(ERROR) << "Can`t find response.GeoObjectCollection.featureMember!";
                        }
                    } else {
                        LOG(ERROR) << "Can`t find response.GeoObjectCollection!";
                    }
                } else {
                    LOG(ERROR) << "Can`t find response!";
                }
            } catch (const std::exception& e) {
                LOG(ERROR) << "Can`t parses recv json: " << e.what();
            }
        }
        curl_easy_cleanup(curl);
    }
    return jgeo;
}
