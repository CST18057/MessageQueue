#ifndef JSONOBJECT_H
#define JSONOBJECT_H
#include <unordered_map>
#include <map>
#include <vector>
#include <memory>
#include <fstream>
#include <functional>
extern const char *ObjectTypeNames[];
enum ObjectType
{
    JSONINT = 0,
    JSONDOUBLE,
    JSONBOOL,
    JSONNULLPTR,
    JSONSTRING,
    JSONDICT,
    JSONARRAY,
    JSONINVALID
};
class JsonObject
{
public:
    JsonObject();
    JsonObject(ObjectType t);
    JsonObject(const JsonObject &jo);
    explicit JsonObject(int pi);
    explicit JsonObject(long long pl);
    explicit JsonObject(const std::string &ps);
    explicit JsonObject(const char *ps);
    explicit JsonObject(double pd);
    explicit JsonObject(bool pb);
    JsonObject(const std::unordered_map<std::string, JsonObject *> &pt);
    JsonObject(const std::vector<JsonObject *> &pv);
    static std::unique_ptr<JsonObject> decoder(std::string &s);
    //采取左闭右开的方式
    static JsonObject *findNextJsonObeject(std::string &s, int &sta, int &end);
    static JsonObject *findNextNumber(std::string &s, int &sta, int &end);
    static JsonObject *findNextString(std::string &s, int &sta, int &end);
    static JsonObject *findNextDict(std::string &s, int &sta, int &end);
    static JsonObject *findNextArray(std::string &s, int &sta, int &end);
    static JsonObject *findNextBool(std::string &s, int &sta, int &end);
    static JsonObject *findNextNullptr(std::string &s, int &sta, int &end);
    static inline int firstNonEmpty(std::string &s, int sta, int end, int step = 1, bool flag = true);
    static int skipEmptyOneChar(std::string &s, int sta, int end, char c);
    static void reportError(const char *s, int x, int y = -1);
    static std::string replace(std::string s, std::string old_s, std::string new_s, int n = 1e9 + 7);
    ObjectType getType();
    const char *getTypeStr();
    long long asInt();
    double asDouble();
    std::string asString();
    std::unordered_map<std::string, JsonObject *> &asDict();
    std::vector<JsonObject *> &asArray();
    bool asBool();
    void *asNullptr();
    static JsonObject *copy(const JsonObject &jo);
    std::string convert(bool reverse = false);
    static std::string convert(std::string s, bool reverse = false);
    std::string json();
    ~JsonObject();
    JsonObject &operator=(const JsonObject &jo);


private:
    ObjectType type;
    union
    {
        bool b;
        long long i;
        double d;
        std::string s;
        std::unordered_map<std::string, JsonObject *> t;
        std::vector<JsonObject *> v;
    };
};

#endif