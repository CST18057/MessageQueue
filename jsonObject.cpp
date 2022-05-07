#include <iostream>
#include <algorithm>
#include <sstream>
#include <typeinfo>
#include <string.h>
#include <memory>
#include <fstream>
#include "jsonObject.h"
using namespace std;

const char *ObjectTypeNames[] = {
    "int",
    "double",
    "bool",
    "nullptr",
    "string",
    "dict",
    "array",
    "invalid"
};

std::string JsonObject::replace(std::string s, std::string old_s, std::string new_s, int n)
{
    int pre = 0, now;
    string res = "";
    while (pre < s.length() && (now = s.find(old_s, pre)) != -1 && n)
    {
        res += s.substr(pre, now - pre);
        res += new_s;
        pre = now + old_s.length();
        n--;
    }
    res += s.substr(pre);
    return res;
}

JsonObjectPtr JsonObject::copy(const JsonObject &jo)
{
    JsonArray pv;
    JsonDict mp;
    switch (jo.type)
    {
    case JSONDOUBLE:
        return JSONOBJECT(jo.d);
        break;
    case JSONINT:
        return JSONOBJECT(jo.i);
        break;
    case JSONBOOL:
        return JSONOBJECT(jo.b);
        break;
    case JSONNULLPTR:
        return JSONOBJECT();
        break;
    case JSONINVALID:
        return JSONOBJECT(JSONINVALID);
        break;
    case JSONSTRING:
        return JSONOBJECT(jo.s);
        break;
    case JSONDICT:
        for (pair<string, JsonObjectPtr > p : jo.t)
        {
            mp.insert(make_pair(p.first, copy(*p.second)));
        }
        return JSONOBJECT(mp);
        break;
    case JSONARRAY:
        for (JsonObjectPtr i : jo.v)
        {
            pv.push_back(copy(*i));
        }
        return JSONOBJECT(pv);
        break;
    default:
        break;
    }
    return JSONOBJECT(JSONINVALID);
}

JsonObject & JsonObject::operator=(const JsonObject &jo)
{
    if (this == &jo)
        return *this;
    JsonArray pv;
    JsonDict mp;
    type = jo.type;
    switch (jo.type)
    {
    case JSONDOUBLE:
        this->d = jo.d;
        break;
    case JSONINT:
        this->i = jo.i;
        break;
    case JSONBOOL:
        this->b = jo.b;
        break;
    case JSONNULLPTR:
        break;
    case JSONSTRING:
        new (&s) std::string(jo.s);
        break;
    case JSONDICT:
        for (pair<string, JsonObjectPtr > p : jo.t)
        {
            mp.insert(make_pair(p.first, copy(*p.second)));
        }
        new (&t) JsonDict(mp);
        break;
    case JSONARRAY:
        for (JsonObjectPtr i : jo.v)
        {
            pv.push_back(copy(*i));
        }
        new (&v) JsonArray(pv);
        break;
    default:
        break;
    }
    return *this;
}

JsonObject::JsonObject(const JsonObject &jo)
{
    JsonArray pv;
    JsonDict mp;
    type = jo.type;
    switch (jo.type)
    {
    case JSONDOUBLE:
        this->d = jo.d;
        break;
    case JSONINT:
        this->i = jo.i;
        break;
    case JSONBOOL:
        this->b = jo.b;
        break;
    case JSONNULLPTR:
        break;
    case JSONSTRING:
        new (&s) std::string(jo.s);
        break;
    case JSONDICT:
        for (pair<string, JsonObjectPtr > p : jo.t)
        {
            mp.insert(make_pair(p.first, copy(*p.second)));
        }
        new (&t) JsonDict(mp);
        break;
    case JSONARRAY:
        for (JsonObjectPtr i : jo.v)
        {
            pv.push_back(copy(*i));
        }
        new (&v) JsonArray(pv);
        break;
    default:
        break;
    }
}

JsonObject::JsonObject() : type(JSONNULLPTR) {}
JsonObject::JsonObject(int pi) : i(pi), type(JSONINT) {}
JsonObject::JsonObject(long long pl) : i(pl), type(JSONINT) {}
JsonObject::JsonObject(const std::string& ps) : type(JSONSTRING)
{
    new (&s) std::string(ps);
}
JsonObject::JsonObject(const char *ps) : type(JSONSTRING)
{
    new (&s) std::string(ps);
}
JsonObject::JsonObject(double pd) : d(pd), type(JSONDOUBLE) {}
JsonObject::JsonObject(bool pb) : b(pb), type(JSONBOOL) {}
JsonObject::JsonObject(const JsonDict& pt) : type(JSONDICT)
{
    new (&t) JsonDict(pt);
}
JsonObject::JsonObject(const JsonArray& pv) : type(JSONARRAY)
{
    new (&v) JsonArray(pv);
}
JsonObject::~JsonObject()
{
    switch (type)
    {
    case JSONSTRING:
        break;
        s.~string();
    case JSONDICT:
        for (pair<string, JsonObjectPtr > p : t)
        {
            // delete p.second;
            p.second.reset();
        }
        t.~JsonDict();
        break;
    case JSONARRAY:
        for (JsonObjectPtr i : v)
        {
            // delete i;
            i.reset();
        }
        v.~JsonArray();
        break;
    default:
        break;
    }
}

ObjectType JsonObject::getType() { return type; }

const char *JsonObject::getTypeStr() { return ObjectTypeNames[(int)type]; }

long long& JsonObject::asInt()
{
    if (type != JSONINT)
        throw "get value type error(int),correct type is " + string(ObjectTypeNames[(int)type]);
    return i;
}
double& JsonObject::asDouble()
{
    if (type != JSONDOUBLE)
        throw "get value type error(double),correct type is " + string(ObjectTypeNames[(int)type]);
    return d;
}
std::string& JsonObject::asString()
{
    if (type != JSONSTRING)
        throw "get value type error(string),correct type is " + string(ObjectTypeNames[(int)type]);
    return s;
}
JsonDict& JsonObject::asDict()
{
    if (type != JSONDICT)
        throw "get value type error(dict),correct type is " + string(ObjectTypeNames[(int)type]);
    return t;
}
JsonArray& JsonObject::asArray()
{
    if (type != JSONARRAY)
        throw "get value type error(array),correct type is " + string(ObjectTypeNames[(int)type]);
    return v;
}
bool& JsonObject::asBool()
{
    if (type != JSONBOOL)
        throw "get value type error(bool),correct type is " + string(ObjectTypeNames[(int)type]);
    return b;
}
void *JsonObject::asNullptr()
{
    if (type != JSONNULLPTR)
        throw "get value type error(nullptr),correct type is " + string(ObjectTypeNames[(int)type]);
    return nullptr;
}

std::string JsonObject::convert(bool reverse)
{
    stringstream ss;
    switch (type)
    {
    case JSONDOUBLE:
        ss << d;
        break;
    case JSONINT:
        ss << i;
        break;
    case JSONBOOL:
        ss << (b ? "true" : "false");
        break;
    case JSONNULLPTR:
        ss << "null";
        break;
    case JSONSTRING:
        ss << s;
        break;
    case JSONINVALID:
        throw "invalid json";
    default:
        break;
    }
    string s = ss.str();
    if (type == JSONSTRING)
    {
        if (reverse)
        {
            s = replace(s, "\\\\", "\\");
            s = replace(s, "\\\"", "\"");
            s = replace(s, "\\r", "\r");
            s = replace(s, "\\n", "\n");
            s = replace(s, "\\b", "\b");
            s = replace(s, "\\f", "\f");
            s = replace(s, "\\t", "\t");
            s = replace(s, "\\/", "/");
        }
        else
        {
            s = replace(s, "\\", "\\\\"); //将反斜杠转为反斜杠反斜杠
            s = replace(s, "\"", "\\\""); //将双引号转为反斜杠双引号
            s = replace(s, "\r", "\\r");
            s = replace(s, "\n", "\\n");
            s = replace(s, "\b", "\\b");
            s = replace(s, "\f", "\\f");
            s = replace(s, "\t", "\\t");
            s = replace(s, "/", "\\/");
            s = "\"" + s + "\"";
        }
    }
    return s;
}

std::string JsonObject::convert(std::string s, bool reverse)
{
    if (reverse)
    {
        s = replace(s, "\\\\", "\\");
        s = replace(s, "\\\"", "\"");
        s = replace(s, "\\r", "\r");
        s = replace(s, "\\n", "\n");
        s = replace(s, "\\b", "\b");
        s = replace(s, "\\f", "\f");
        s = replace(s, "\\t", "\t");
        s = replace(s, "\\/", "/");
    }
    else
    {
        s = replace(s, "\\", "\\\\"); //将反斜杠转为反斜杠反斜杠
        s = replace(s, "\"", "\\\""); //将双引号转为反斜杠双引号
        s = replace(s, "\r", "\\r");
        s = replace(s, "\n", "\\n");
        s = replace(s, "\b", "\\b");
        s = replace(s, "\f", "\\f");
        s = replace(s, "\t", "\\t");
        s = replace(s, "/", "\\/");
        s = "\"" + s + "\"";
    }
    return s;
}

std::string JsonObject::json()
{
    string res = "";
    switch (type)
    {
    case JSONDOUBLE:
    case JSONINT:
    case JSONBOOL:
    case JSONNULLPTR:
    case JSONSTRING:
        return convert();
        break;
    case JSONDICT:
        for (pair<string, JsonObjectPtr > p : t)
        {
            if (res != "")
                res += ",";
            res += JsonObject(p.first).convert() + ":" + p.second->json();
        }
        return "{" + res + "}";
        break;
    case JSONARRAY:
        for (JsonObjectPtr p : v)
        {
            if (res != "")
                res += ",";
            res += p->json();
        }
        return "[" + res + "]";
        break;
    case JSONINVALID:
        throw "invalid json";
    default:
        return "";
    }
}

JsonObjectPtr JsonObject::decoder(const std::string &s)
{
    if (s.length() == 0)
        return JSONOBJECT(JSONINVALID);
    try
    {
        int sta = 0, end = s.length();
        return findNextDict(s, sta, end);
        // JsonObject* temp = findNextDict(s, sta, end);
        // JsonObject res = *temp;
        // delete temp;
        // return res;
    }
    catch (char *s)
    {
        std::cout << "catch error char*" << endl;
        std::cout << "msg:" << s << endl;
        delete s;
        return JSONOBJECT(JSONINVALID);
    }
    catch (const char *s)
    {
        std::cout << "catch error const char*" << endl;
        std::cout << "msg:" << s << endl;
        return JSONOBJECT(JSONINVALID);
    }
    catch (string s)
    {
        std::cout << "catch error string" << endl;
        std::cout << "size:" << s.length() << "msg:" << s << endl;
        return JSONOBJECT(JSONINVALID);
    }
}

int JsonObject::firstNonEmpty(const std::string &s, int sta, int end, int step, bool flag)
{
    while (sta < end && (isspace(s[sta]) > 0) == flag)
        sta++;
    return sta;
}

int JsonObject::skipEmptyOneChar(const std::string &s, int sta, int end, char c)
{
    bool flag = false;
    while (sta < end)
    {
        if (!isspace(s[sta]))
        {
            if (s[sta] == c)
            {
                // std::cout << sta << " " << s[sta] << " " << flag << endl;
                if (!flag)
                    flag = true;
                else
                    break;
            }
            else
            {
                break;
            }
        }
        sta++;
    }
    return sta;
}

void JsonObject::reportError(const char *s, int x, int y)
{
    char *res = new char[100];
    if (y == -1)
        sprintf(res, s, x);
    else
        sprintf(res, s, x, y);
    // std::cout << "error msg:" << res << endl;
    throw res;
}

JsonObjectPtr JsonObject::findNextJsonObeject(const std::string &s, int &sta, int &end)
{
    int now = firstNonEmpty(s, sta, end);
    sta = now;
    if (now == end)
        return JSONOBJECT(JSONINVALID);
    char c = s[sta];
    // std::cout << "start char:" << c << endl;
    JsonObjectPtr temp = nullptr;
    if (c == '{')
        return findNextDict(s, sta, end);
    else if (c == '[')
        return findNextArray(s, sta, end);
    else if (c == '\"')
        return findNextString(s, sta, end);
    else if (c == '-' || c >= '0' && c <= '9')
        return findNextNumber(s, sta, end);
    else if (c == 't' || c == 'f')
        return findNextBool(s, sta, end);
    else if (c == 'n')
        return findNextNullptr(s, sta, end);
    else
        // throw "invalid json";
        return JSONOBJECT(JSONINVALID);
    // if (temp->getType() == JSONINVALID)
    // {
    //     return
    // reportError("type error:jsonobject,invalid json in char:{%d->%d}", sta, end);
    // }
    // std::cout << temp.getTypeStr() << endl;
    return temp;
}

JsonObjectPtr JsonObject::findNextNumber(const std::string &s, int &sta, int &end)
{
    // cout << "number" << endl;
    int now = firstNonEmpty(s, sta, end);
    if (now >= end)
        return JSONOBJECT(JSONINVALID);
    long long l;
    double d;
    char *pos;
    const char *ts = strdup(s.substr(sta, min(end - sta, 30)).c_str());
    // std::cout << ts << endl;
    d = strtod(ts, &pos);
    // std::cout << d << endl;
    int k = pos - ts;
    // std::cout << "number size:" << k << endl;
    if (k == 0)
        return JSONOBJECT(JSONINVALID);
    bool onlyDigit = true;
    for (int i = 0; i < k; i++)
        if (s[sta + i] != '-' && !isdigit(s[sta + i]))
        {
            onlyDigit = false;
            break;
        }
    sta += k;
    //纯数字且长度小于十八位考虑将其转为long long
    if (onlyDigit && k + 1 < 18)
    {
        l = strtoll(ts, &pos, 0);
        delete ts;
        return JSONOBJECT(l);
    }
    else
    {
        delete ts;
        return JSONOBJECT(d);
    }
}
JsonObjectPtr JsonObject::findNextBool(const std::string &s, int &sta, int &end)
{
    // cout << "bool" << endl;
    int now = firstNonEmpty(s, sta, end);
    if (now >= end)
        return JSONOBJECT(JSONINVALID);
    if (end - sta >= 4 && s.substr(sta, 4) == "true")
    {
        sta += 4;
        return JSONOBJECT(true);
    }
    else if (end - sta >= 5 && s.substr(sta, 5) == "false")
    {
        sta += 5;
        return JSONOBJECT(false);
    }
    return JSONOBJECT(JSONINVALID);
}
JsonObjectPtr JsonObject::findNextArray(const std::string &s, int &sta, int &end)
{
    int now = firstNonEmpty(s, sta, end);
    if (now >= end)
        return JSONOBJECT(JSONINVALID);
    sta = now;
    if (s[now] != '[')
        return JSONOBJECT(JSONINVALID);
    sta++;
    JsonObjectPtr temp;
    JsonArray vj;
    while (true)
    {
        sta = firstNonEmpty(s, sta, end);
        // cout << "array element begin:" << sta << endl;
        if (sta == end)
        {
            for (JsonObjectPtr i : vj)
            {
                // delete i;
                i.reset();
            }
            return JSONOBJECT(JSONINVALID);
            // reportError("type error:array,invalid json in char:{%d->%d}", sta, end);
        }
        if (s[sta] == ']')
        {
            sta++;
            break;
        }
        temp = findNextJsonObeject(s, sta, end);
        // cout << "array element:" << temp->getType() << endl;
        // cout << "array element:" << temp->getTypeStr() << endl;
        if (temp->getType() == JSONINVALID)
        {
            for (JsonObjectPtr i : vj)
            {
                // delete i;
                i.reset();
            }
            return temp;
            // reportError("type error:array,invalid json in char:{%d->%d}", sta, end);
        }
        // JsonObjectPtr value = JSONOBJECT(temp);
        // vj.push_back(value);
        vj.push_back(temp);
        // cout << "array size:" << vj.size() << endl;
        sta = skipEmptyOneChar(s, sta, end, ',');
    }
    return JSONOBJECT(vj);
}
JsonObjectPtr JsonObject::findNextNullptr(const std::string &s, int &sta, int &end)
{
    int now = firstNonEmpty(s, sta, end);
    sta = now;
    if (now >= end)
        return JSONOBJECT(JSONINVALID);
    if (end - sta >= 4 && s.substr(sta, 4) == "null")
    {
        sta += 4;
        return JSONOBJECT();
    }
    return JSONOBJECT(JSONINVALID);
}
JsonObjectPtr JsonObject::findNextDict(const std::string &s, int &sta, int &end)
{
    int now = firstNonEmpty(s, sta, end);
    if (now >= end)
        return JSONOBJECT(JSONINVALID);
    sta = now;
    // std::cout << sta << " " << end << endl;
    if (s[now] != '{')
        return JSONOBJECT(JSONINVALID);
    sta++;
    JsonObjectPtr temp;
    JsonDict mp;
    while (true)
    {
        sta = firstNonEmpty(s, sta, end);
        // std::cout << "dict range:" << sta << " " << end << endl;
        if (sta >= end)
        {
            for (pair<string, JsonObjectPtr > p : mp)
            {
                // delete p.second;
                p.second.reset();
            }
            return JSONOBJECT(JSONINVALID);
            // reportError("type error:dict,invalid json in char:{%d->%d}", sta, end);
        }
        if (s[sta] == '}')
        {
            sta++;
            break;
        }
        // std::cout << "find string key" << endl;
        temp = findNextString(s, sta, end);
        // std::cout << "get key:" << temp->asString() << endl;
        if (temp->getType() == JSONINVALID)
        {
            for (pair<string, JsonObjectPtr > p : mp)
            {
                // delete p.second;
                p.second.reset();
            }
            return temp;
            // reportError("type error:dict,invalid json in char:{%d->%d}", sta, end);
        }
        string key = temp->asString();
        // delete temp;
        temp.reset();
        sta = skipEmptyOneChar(s, sta, end, ':');
        // std::cout << "find value" << endl;
        temp = findNextJsonObeject(s, sta, end);
        // std::cout << "get value:" << temp->getTypeStr() << endl;
        // cout << temp->json() << endl;
        if (temp->getType() == JSONINVALID)
        {
            for (pair<string, JsonObjectPtr > p : mp)
            {
                // delete p.second;
                p.second.reset();
            }
            return temp;
            // reportError("type error:dict,invalid json in char:{%d->%d}", sta, end);
        }
        mp[key] = temp;
        // std::cout << "end dict range:" << sta << " " << end << endl;
        sta = skipEmptyOneChar(s, sta, end, ',');
    }
    return JSONOBJECT(mp);
}
JsonObjectPtr JsonObject::findNextString(const std::string &s, int &sta, int &end)
{
    // cout << "string" << endl;
    int now = firstNonEmpty(s, sta, end);
    // std::cout << "string:" << now << "|" << s[now] << endl;
    if (now >= end)
        return JSONOBJECT(JSONINVALID);
    sta = now;
    if (s[sta] != '\"')
    {
        // printf("invalid char : %c\n", s[sta]);
        // cout << s.substr(sta) << endl;
        reportError("type error:string,invalid json in char:{%d->%d}", sta, end);
    }
    int l = now + 1, r = l;
    while (r < end)
    {
        if (s[r] == '\"' && s[r - 1] != '\\')
            break;
        r++;
    }
    if (r >= end)
        reportError("type error:string,invalid json in char:{%d->%d}", sta, end);
    // std::cout << "string range:" << l << " " << r << endl;
    sta = r + 1;
    return JSONOBJECT(s.substr(l, r - l));
}