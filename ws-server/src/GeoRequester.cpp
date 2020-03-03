#include <iostream>

#include <curl/curl.h>
#include <stdio.h>

#include "Log.hpp"
#include "GeoRequester.hpp"


using namespace server;


static std::string UrlEncode(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        std::string::value_type c = (*i);
        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) or c == '-' or c == '_' or c == '.' or c == '~') {
            escaped << c;
            continue;
        }
        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }
    return escaped.str();
}


static size_t RecvFn(char* data, size_t size, size_t nmemb, void* buf) {
    reinterpret_cast<std::string*>(buf)->append(data, size * nmemb);
    return size * nmemb;
}


static std::string __debug_str;

static int DebugCallback(CURL *handle,
                   curl_infotype type,
                   char *data,
                   size_t size,
                   void *userptr) {
    std::string type_str;
    switch (type) {
    case CURLINFO_TEXT:
        type_str = "TEXT";
        break;
    case CURLINFO_HEADER_IN:    /* 1 */
        type_str = "HEADER_IN";
        break;
    case CURLINFO_HEADER_OUT:   /* 2 */
        type_str = "HEADER_OUT";
        break;
    case CURLINFO_DATA_IN:      /* 3 */
        type_str = "DATA_IN";
        break;
    case CURLINFO_DATA_OUT:     /* 4 */
        type_str = "DATA_OUT";
        break;
    case CURLINFO_SSL_DATA_IN:  /* 5 */
        type_str = "SSL_DATA_IN";
        break;
    case CURLINFO_SSL_DATA_OUT: /* 6 */
        type_str = "SSL_DATA_OUT";
        break;
    case CURLINFO_END:
        break;
    }
    if (type == CURLINFO_TEXT or
        type == CURLINFO_HEADER_OUT or
        type == CURLINFO_DATA_OUT or
        type == CURLINFO_SSL_DATA_OUT or
        type == CURLINFO_HEADER_IN or
        type == CURLINFO_DATA_IN or
        type == CURLINFO_SSL_DATA_IN) {
        std::string str(data, data + size);
        __debug_str += "[" + type_str + "] - " + str;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Json GeoRequester::parseResult(const std::string& body_buf, Json& jgeo) {
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
    return jgeo;
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
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(("curl -X GET \"https://geocode-maps.yandex.ru/1.x/?sco=longlat&lang=ru_RU&format=json&apikey=" +
                       _api_key + "&geocode=" + UrlEncode(coll) + "\"").c_str(), "r");
    Json jgeo = {
        {"addr_line", coll},
        {"point", {0.0, 0.0}}
    };
    if (pipe) {
        try {
            while (fgets(buffer, sizeof buffer, pipe) not_eq nullptr) {
                result += buffer;
            }
        } catch (...) {
            pclose(pipe);
            throw;
        }
        pclose(pipe);
        //result = result.substr(0, result.rfind("}") + 1);
        LOG(TRACE) << "\"" << result << "\"";
        parseResult(result, jgeo);
    } else {
        LOG(ERROR) << "POPEN is failed!";
    }
    return jgeo;
}
