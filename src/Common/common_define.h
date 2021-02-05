#ifndef COMMON_DEFINE
#define COMMON_DEFINE

#include <QString>
#include <list>
#include <QStringList>

using namespace std;

inline void csg_split(const string &str,const string &sep, list<string> &strList)
{
    strList.clear();
    string::size_type nPos = 0;
    while (nPos <= str.length())
    {
        string::size_type pos = str.find(sep, nPos);
        if (string::npos == pos)
        {
            strList.push_back(str.substr(nPos,str.length()-1));
            break;
        }
        strList.push_back(str.substr(nPos,pos-nPos));
        nPos = pos+1;
    }
}

// 获取编译时间 格式：2017/11/03 8:43
inline string csg_getCompileDatetime()
{
    string date = __DATE__;
    string time = __TIME__;
    string year;   // 年
    string month;  // 月
    string day;    // 日
    string HH;     // 时
    string mm;     // 分
    string ss;     // 秒

    list<string> dateList;
    csg_split(date," ",dateList);

    list<string>::iterator itr;
    for(itr = dateList.begin(); itr != dateList.end(); )
    {
        if( (*itr).empty() )
        {
            itr = dateList.erase(itr);
        }
        else
        {
            ++itr;
        }
    }

    if(dateList.size() == 3)
    {
        list<string>::iterator iter = dateList.begin();

        string m = *iter;
        if(m == "Jan") {
            month = "01";
        }
        else if(m == "Feb") {
            month = "02";
        }
        else if(m == "Mar") {
            month = "03";
        }
        else if(m == "Apr") {
            month = "04";
        }
        else if(m == "May") {
            month = "05";
        }
        else if(m == "Jun") {
            month = "06";
        }
        else if(m == "Jul") {
            month = "07";
        }
        else if(m == "Aug") {
            month = "08";
        }
        else if(m == "Sep") {
            month = "09";
        }
        else if(m == "Oct") {
            month = "10";
        }
        else if(m == "Nov") {
            month = "11";
        }
        else if(m == "Dec") {
            month = "12";
        }

        ++iter;
        day = *(iter);
        if(day.size() == 1)
        {
            day.insert(0,"0");
        }

        ++iter;
        year = *(iter);
    }
    else
    {
        year = "yyyy";
        month = "MM";
        day = "dd";
    }

    list<string> timeList;
    csg_split(time,":",timeList);

    if(timeList.size() == 3)
    {
        list<string>::iterator iter = timeList.begin();
        HH = *iter;
        mm = *(++iter);
        ss = *(++iter);
    }
    else
    {
        HH = "HH";
        mm = "mm";
        ss = "ss";
    }

    return year + "/" + month + "/" + day + " " + HH + ":" + mm + ":" + ss;
}

// 获取软件版本号 示例 1.0
inline string csg_getSoftwareVersion()
{
    return string("V1.0");
}

inline string csg_debug_release()
{
#ifdef QT_DEBUG
    return "Debug";
#else
    return "Release";
#endif
}

#endif // COMMON_DEFINE

