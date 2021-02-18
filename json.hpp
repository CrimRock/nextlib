#ifndef NEXTLIB_JSON_HPP
#define NEXTLIB_JSON_HPP

#include <string>

namespace Next
{
    class Json {
    public:
        Json() {
            content = "{}";
        }

        void Add(std::string key, Json *subJson) {
            PrivateAdd(key, subJson->toString(), false);
        }

        void Add(std::string key, std::string value) {
            PrivateAdd(key, value, true);
        }

        void Add(std::string key, double value) {
            PrivateAdd(key, std::to_string(value), false);
        }

        void Add(std::string key, bool value, bool isBool) {
            if (value == true)
                PrivateAdd(key, "true", false);
            else
                PrivateAdd(key, "false", false);
        }

        std::string toString() {
            return content;
        }
    private:
        std::string content;

        void PrivateAdd(std::string key, std::string value, bool stringType) {
            std::string start = "";
            for (int i = 0; i < content.length(); i++) {
                if (content.length() != i + 1) {
                    start += content[i];
                }
            }
            if (content != "{}") {
                start = start + ",";
            }

            if (stringType == true) {
                value = "\"" + value + "\"";
            }

            content = start + "\"" + key + "\":" + value + "}";
        }
    };
}

#endif // NEXTLIB_JSON_HPP