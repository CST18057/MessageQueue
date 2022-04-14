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

class JsonObject;
using JsonObjectPtr = std::shared_ptr<JsonObject>;
using JsonDict = std::unordered_map<std::string, JsonObjectPtr>;
using JsonArray = std::vector<JsonObjectPtr>;
#define JSONOBJECT(x) std::make_shared<JsonObject>(x)

class JsonObject
{
public:
    JsonObject();
    JsonObject(const JsonObject &jo);
    explicit JsonObject(int pi);
    explicit JsonObject(long long pl);
    explicit JsonObject(const std::string &ps);
    explicit JsonObject(const char *ps);
    explicit JsonObject(double pd);
    explicit JsonObject(bool pb);
    JsonObject(const JsonDict &pt);
    JsonObject(const JsonArray &pv);
    static JsonObjectPtr decoder(const std::string &s);
    //采取左闭右开的方式
    static JsonObjectPtr findNextJsonObeject(const std::string &s, int &sta, int &end);
    static JsonObjectPtr findNextNumber(const std::string &s, int &sta, int &end);
    static JsonObjectPtr findNextString(const std::string &s, int &sta, int &end);
    static JsonObjectPtr findNextDict(const std::string &s, int &sta, int &end);
    static JsonObjectPtr findNextArray(const std::string &s, int &sta, int &end);
    static JsonObjectPtr findNextBool(const std::string &s, int &sta, int &end);
    static JsonObjectPtr findNextNullptr(const std::string &s, int &sta, int &end);
    static inline int firstNonEmpty(const std::string &s, int sta, int end, int step = 1, bool flag = true);
    static int skipEmptyOneChar(const std::string &s, int sta, int end, char c);
    static void reportError(const char *s, int x, int y = -1);
    static std::string replace(std::string s, std::string old_s, std::string new_s, int n = 1e9 + 7);
    ObjectType getType();
    const char *getTypeStr();
    long long &asInt();
    double &asDouble();
    std::string &asString();
    JsonDict &asDict();
    JsonArray &asArray();
    bool &asBool();
    void *asNullptr();
    static JsonObjectPtr copy(const JsonObject &jo);
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
        JsonDict t;
        JsonArray v;
    };
};

#endif