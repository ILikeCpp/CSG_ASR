#include "SinoVoiceASRService.h"
#include <time.h>
#include <QDebug>
#include <iostream>
#include <sstream>

using namespace std;

#ifdef __LINUX__
#include <string.h>
#define stricmp strcasecmp
#else
#define stricmp _stricmp
#endif

struct
{
    const char* pszName;
    const char* pszComment;
}g_sStatus[] =
{
    {"RECORDER_EVENT_BEGIN_RECORD",         "录音开始"},
    {"RECORDER_EVENT_HAVING_VOICE",         "听到声音 检测到始端的时候会触发该事件"},
    {"RECORDER_EVENT_NO_VOICE_INPUT",       "没有听到声音"},
    {"RECORDER_EVENT_BUFF_FULL",            "缓冲区已填满"},
    {"RECORDER_EVENT_END_RECORD",           "录音完毕（自动或手动结束）"},
    {"RECORDER_EVENT_BEGIN_RECOGNIZE",      "开始识别"},
    {"RECORDER_EVENT_RECOGNIZE_COMPLETE",   "识别完毕"},
    {"RECORDER_EVENT_ENGINE_ERROR",         "引擎出错"},
    {"RECORDER_EVENT_DEVICE_ERROR",         "设备出错"},
    {"RECORDER_EVENT_MALLOC_ERROR",         "分配空间失败"},
    {"RECORDER_EVENT_INTERRUPTED",          "内部错误"},
    {"RECORDER_EVENT_PERMISSION_DENIED",    "内部错误"},
    {"RECORDER_EVENT_TASK_FINISH",          "识别任务结束"},
    {"RECORDER_EVENT_RECOGNIZE_PROCESS",    "识别中间状态"}
};

SinoVoiceASRService::SinoVoiceASRService():
    m_error(""),m_GrammarId(-1)
{
    Init();
}

SinoVoiceASRService::~SinoVoiceASRService()
{
    Uninit();
    qDebug() << "SinoVoiceASRService::~SinoVoiceASRService";
}

bool SinoVoiceASRService::startService()
{
    RECORDER_ERR_CODE eRet = RECORDER_ERR_NONE;

    AccountInfo *account_info = AccountInfo::GetInstance();
    string startConfig = "";
    startConfig += "capkey=" + account_info->cap_key();
    startConfig += ",audioformat=pcm16k16bit";
    //startConfig += ",domain=qwdz,intention=qwmap;music,needcontent=no";
    //startConfig     += ",realTime=rt";
    startConfig += ",continuous=yes";

    if ( m_RecogMode == kRecogModeGrammar )
    {
        char chTmp[32] = {0};
        sprintf(chTmp,",grammarid=%d",m_GrammarId);
        startConfig += chTmp;
    }

    eRet = hci_asr_recorder_start(startConfig.c_str(),"");
    if (RECORDER_ERR_NONE != eRet)
    {
        ostringstream ostr;
        ostr << "开始录音失败,错误码 eRet:" << eRet;
        m_error = ostr.str();
        return false;
    }
    qDebug() << "hci_asr_recorder_start sucessed!";
    return true;
}

bool SinoVoiceASRService::stopService()
{
    RECORDER_ERR_CODE eRet = hci_asr_recorder_cancel();
    if (RECORDER_ERR_NONE != eRet)
    {
        ostringstream ostr;
        ostr << "终止录音失败,错误码 eRet:" << eRet;
        m_error = ostr.str();
        return false;
    }
    qDebug() << "hci_asr_recorder_cancel sucessed!";
    return true;
}

std::string SinoVoiceASRService::errorString()
{
    return m_error;
}

bool SinoVoiceASRService::CheckAndUpdataAuth()
{
    //获取过期时间
    int64 nExpireTime;
    int64 nCurTime = (int64)time( NULL );
    HCI_ERR_CODE errCode = hci_get_auth_expire_time( &nExpireTime );
    if( errCode == HCI_ERR_NONE )
    {
        //获取成功则判断是否过期
        if( nExpireTime > nCurTime )
        {
            //没有过期
            qDebug() << "auth can use continue";
            return true;
        }
    }

    //获取过期时间失败或已经过期
    //手动调用更新授权
    errCode = hci_check_auth();
    if( errCode == HCI_ERR_NONE )
    {
        //更新成功
        qDebug() << "check auth success";
        return true;
    }
    else
    {
        //更新失败
        ostringstream ostr;
        ostr << "check auth return  errCode:" << errCode << "  errInfo:" << hci_get_error_info(errCode);
        m_error = ostr.str();
        return false;
    }
}

//获取capkey属性
void SinoVoiceASRService::GetCapkeyProperty(const string&cap_key,AsrRecogType & type,AsrRecogMode &mode)
{
    HCI_ERR_CODE errCode = HCI_ERR_NONE;
    CAPABILITY_ITEM *pItem = NULL;

    // 枚举所有的asr能力
    CAPABILITY_LIST list = {0};
    if ((errCode = hci_get_capability_list("asr", &list))!= HCI_ERR_NONE)
    {
        // 没有找到相应的能力。
        return;
    }

    // 获取asr能力配置信息。
    for (int i = 0; i < list.uiItemCount; i++)
    {
        if (list.pItemList[i].pszCapKey != NULL && stricmp(list.pItemList[i].pszCapKey, cap_key.c_str()) == 0)
        {
            pItem = &list.pItemList[i];
            break;
        }
    }

    // 没有获取相应能力配置，返回。
    if (pItem == NULL || pItem->pszCapKey == NULL)
    {
        hci_free_capability_list(&list);
        return;
    }


    if (strstr(pItem->pszCapKey, "cloud") != NULL)
    {
        type = kRecogTypeCloud;
    }
    else
    {
        type = kRecogTypeLocal;
    }

    if (strstr(pItem->pszCapKey, "freetalk") != NULL)
    {
        mode = kRecogModeFreetalk;
    }
    else if (strstr(pItem->pszCapKey, "grammar") != NULL)
    {
        mode = kRecogModeGrammar;
    }
    else
    {
        mode = kRecogModeUnkown;
    }

    hci_free_capability_list(&list);

    return;
}

bool SinoVoiceASRService::Init()
{
    std::string strErrorMessage;

    // 获取AccountInfo单例
    AccountInfo *account_info = AccountInfo::GetInstance();
    // 账号信息读取
    string account_info_file = "../../testdata/AccountInfo.txt";
    bool account_success = account_info->LoadFromFile(account_info_file);
    if (!account_success)
    {
        string strErrorMessage("AccountInfo read from %s failed\n");
        strErrorMessage += account_info_file;
        m_error = strErrorMessage;
        return false;
    }

    // SYS初始化
    HCI_ERR_CODE errCode = HCI_ERR_NONE;
    // 配置串是由"字段=值"的形式给出的一个字符串，多个字段之间以','隔开。字段名不分大小写。
    string init_config = "";
    init_config += "appKey=" + account_info->app_key();              //灵云应用序号
    init_config += ",developerKey=" + account_info->developer_key(); //灵云开发者密钥
    init_config += ",cloudUrl=" + account_info->cloud_url();         //灵云云服务的接口地址
    init_config += ",authpath=" + account_info->auth_path();         //授权文件所在路径，保证可写
    init_config += ",logfilepath=" + account_info->logfile_path();   //日志的路径
    init_config += ",logfilesize=1024000,loglevel=5";
    // 其他配置使用默认值，不再添加，如果想设置可以参考开发手册
    errCode = hci_init( init_config.c_str() );
    if( errCode != HCI_ERR_NONE )
    {
        std::ostringstream ostr;
        ostr << "hci_init return errCode:" << errCode << " errInfo:" << hci_get_error_info(errCode);
        m_error = ostr.str();
        return false;
    }
    qDebug() << "hci_init success\n";


    // 检测授权,必要时到云端下载授权。此处需要注意的是，这个函数只是通过检测授权是否过期来判断是否需要进行
    // 获取授权操作，如果在开发调试过程中，授权账号中新增了灵云sdk的能力，请到hci_init传入的authPath路径中
    // 删除HCI_AUTH文件。否则无法获取新的授权文件，从而无法使用新增的灵云能力。
    if (!CheckAndUpdataAuth())
    {
        hci_release();
        string strErrorMessage("CheckAndUpdateAuth failed");
        m_error = strErrorMessage;
        return false;
    }

    // capkey属性获取
    m_RecogType = kRecogTypeUnkown;
    m_RecogMode = kRecogModeUnkown;
    GetCapkeyProperty(account_info->cap_key(),m_RecogType,m_RecogMode);

    if( m_RecogType == kRecogTypeCloud && m_RecogMode == kRecogModeGrammar )
    {
        // 云端语法暂时不支持实时识别
        hci_release();
        string strErrorMessage("Recorder not support cloud grammar, init failed");
        m_error = strErrorMessage;
        return false;
    }

    RECORDER_ERR_CODE eRet = RECORDER_ERR_UNKNOWN;
    RECORDER_CALLBACK_PARAM call_back;
    memset( &call_back, 0, sizeof(RECORDER_CALLBACK_PARAM) );
    call_back.pvStateChangeUsrParam		= this;
    call_back.pvRecogFinishUsrParam		= this;
    call_back.pvErrorUsrParam			= this;
    call_back.pvRecordingUsrParam		= this;
    call_back.pvRecogProcessParam		= this;
    call_back.pfnStateChange	= SinoVoiceASRService::RecordEventChange;
    call_back.pfnRecogFinish	= SinoVoiceASRService::RecorderRecogFinish;
    call_back.pfnError			= SinoVoiceASRService::RecorderErr;
    call_back.pfnRecording		= SinoVoiceASRService::RecorderRecordingCallback;
    call_back.pfnRecogProcess   = SinoVoiceASRService::RecorderRecogProcess;

    //{
    //	// TODO
    //	while(true)
    //	{
    //		RECORDER_ERR_CODE errCode;
    //		string strConfig = string("initCapKeys=asr.local.grammar.v4,dataPath=") + account_info->data_path();
    //		errCode = hci_asr_recorder_init(strConfig.c_str(), &call_back);
    //		assert(RECORDER_ERR_NONE == errCode);

    //		strConfig = "capKey=asr.local.grammar.v4,grammarType=wordlist,audioFormat=pcm16k16bit,isFile=yes"; // + getLocalGrammarConfig();
    //		string grammarDataPath = "../../testdata/wordlist_utf8.txt";
    //		errCode = hci_asr_recorder_start(strConfig.c_str(), grammarDataPath.c_str());
    //		assert(RECORDER_ERR_NONE == errCode);

    //		Sleep(5000);
    //		errCode = hci_asr_recorder_release();
    //		assert(RECORDER_ERR_NONE == errCode);
    //	}
    //}

    string initConfig = "initCapkeys=" + account_info->cap_key();
    initConfig        += ",dataPath=" + account_info->data_path();
    //string initConfig = "dataPath=" + account_info->data_path();
    //initConfig      += ",encode=speex";
    //initConfig		+= ",initCapkeys=asr.local.grammar";			      //初始化本地引擎

    if(m_RecogMode == kRecogModeGrammar)
    {
        initConfig += "/grammar";
    }
    else if (m_RecogMode == kRecogModeFreetalk)
    {
        initConfig += "/freetalk";
    }

    eRet = hci_asr_recorder_init( initConfig.c_str(), &call_back);
    if (eRet != RECORDER_ERR_NONE)
    {
        hci_release();

        ostringstream ostr;
        ostr << "录音机初始化失败,错误码:" << eRet;
        m_error = ostr.str();
        return false;
    }
    m_GrammarId = -1;
    if (m_RecogMode == kRecogModeGrammar)
    {
        string grammarFile = account_info->test_data_path() + "/csg_voice.gram";
        if (m_RecogType == kRecogTypeLocal)
        {
            string strLoadGrammarConfig = "grammarType=jsgf,isFile=yes,capkey=" + account_info->cap_key();
            eRet = hci_asr_recorder_load_grammar(strLoadGrammarConfig.c_str() , grammarFile.c_str(), &m_GrammarId );
            if( eRet != RECORDER_ERR_NONE )
            {
                hci_asr_recorder_release();
                hci_release();

                ostringstream ostr;
                ostr << "载入语法文件失败,错误码:" << eRet;
                m_error = ostr.str();
                return false;
            }
            EchoGrammarData(grammarFile);
        }
        else
        {
            // 如果是云端语法识别，需要开发者通过开发者社区自行上传语法文件，并获得可以使用的ID。
            // m_GrammarId = 2;
        }
    }

    return true;
}

bool SinoVoiceASRService::Uninit(void)
{
    HCI_ERR_CODE eRet = HCI_ERR_NONE;
    // 如果是本地语法识别，则需要释放语法资源
    if( m_RecogType == kRecogTypeLocal && m_RecogMode == kRecogModeGrammar )
    {
        hci_asr_recorder_unload_grammar( m_GrammarId );
    }
    RECORDER_ERR_CODE eRecRet;
    eRecRet = hci_asr_recorder_release();
    if(eRecRet != RECORDER_ERR_NONE)
    {
        return false;
    }
    eRet = hci_release();

    AccountInfo::ReleaseInstance();

    qDebug() << "hci_release eRet:" << eRet;

    return eRet == HCI_ERR_NONE;
}

void SinoVoiceASRService::EchoGrammarData(const string &grammarFile)
{
    FILE* fp = NULL;
    fopen_s(&fp, grammarFile.c_str(), "rt" );
    if( fp == NULL )
    {
        string strErrorMessage("打开语法文件失败:");
        strErrorMessage += grammarFile;
        qDebug() << strErrorMessage.c_str();
        return;
    }

    unsigned char szBom[3];
    fread( szBom, 3, 1, fp );
    // 若有bom头，则清除，没有则当前位置回到头部
    if( !( szBom[0] == 0xef && szBom[1] == 0xbb && szBom[2] == 0xbf ) )
    {
        fseek( fp, 0, SEEK_SET );
    }

    string grammarData = "";
    char szData[1024] = {0};
    while( fgets( szData, 1024, fp ) != NULL )
    {
//        unsigned char* pszGBK = NULL;
//        HciExampleComon::UTF8ToGBK( (unsigned char*)szData, &pszGBK);
//        grammarData += (char*)pszGBK;
//        HciExampleComon::FreeConvertResult( pszGBK );
        grammarData += (char*)szData;
//        grammarData += "\r\n";
    }

    fclose( fp );
    qDebug() << grammarData.c_str();
    return;
}

void SinoVoiceASRService::RecordEventChange(RECORDER_EVENT eRecorderEvent, void *pUsrParam)
{
    SinoVoiceASRService *cb = static_cast<SinoVoiceASRService*>(pUsrParam);

    if(eRecorderEvent == RECORDER_EVENT_BEGIN_RECOGNIZE)
    {
        //do noting
    }
    if(eRecorderEvent == RECORDER_EVENT_END_RECORD)
    {
        //do noting
    }

    string strMessage(g_sStatus[eRecorderEvent].pszComment);
    qDebug() << strMessage.c_str();
}

void SinoVoiceASRService::RecorderRecogFinish(
                                       RECORDER_EVENT eRecorderEvent,
                                       ASR_RECOG_RESULT *psAsrRecogResult,
                                       void *pUsrParam)
{
//	CString strMessage = "";

//    CRecorder_ExampleDlg *dlg = (CRecorder_ExampleDlg*)pUsrParam;
//	if(eRecorderEvent == RECORDER_EVENT_RECOGNIZE_COMPLETE)
//	{
//		char buff[32];
//		clock_t endClock = clock();

//		strMessage.AppendFormat( "识别时间:%d", (int)endClock - (int)dlg->m_startClock );

//		dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
//	}

//    strMessage = "";
//    if( psAsrRecogResult->uiResultItemCount > 0 )
//    {
//		//得分不得低于20分，高于此得分的结果才进行打印。
//		const int minUiScore = 20;
//		if (psAsrRecogResult->psResultItemList[0].uiScore > minUiScore)
//		{
//			unsigned char* pucUTF8 = NULL;
//			HciExampleComon::UTF8ToGBK((unsigned char*)psAsrRecogResult->psResultItemList[0].pszResult, &pucUTF8);
//			strMessage.AppendFormat("识别结果: %s", pucUTF8);

//			//判断开始或者结束
//			CString strStartEnd = "";
//			CString strStart = ""; strStart.AppendFormat("开始录音");
//			CString strEnd = ""; strEnd.AppendFormat("结束录音");
//			strStartEnd.AppendFormat("%s", pucUTF8);

//			if (0 == strStartEnd.Compare(strStart))
//			{
//				dlg->setRecordingFlag(TRUE);
//			}
//			else if (0 == strStartEnd.Compare(strEnd))
//			{
//				dlg->setRecordingFlag(FALSE);
//			}

//			HciExampleComon::FreeConvertResult(pucUTF8);
//			pucUTF8 = NULL;
//		}
//    }
//    else
//    {
//        strMessage.AppendFormat( "*****无识别结果*****" );
//    }

//	dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
}

void SinoVoiceASRService::RecorderRecogProcess(
                                        RECORDER_EVENT eRecorderEvent,
                                        ASR_RECOG_RESULT *psAsrRecogResult,
                                        void *pUsrParam)
{
//    CRecorder_ExampleDlg *dlg = (CRecorder_ExampleDlg*)pUsrParam;
//    CString strMessage = "";
//    if( psAsrRecogResult->uiResultItemCount > 0 )
//    {
//		//得分不得低于20分，高于此得分的结果才进行打印。
//		const int minUiScore = 20;
//		if (psAsrRecogResult->psResultItemList[0].uiScore > minUiScore)
//		{
//			unsigned char* pucUTF8 = NULL;
//			HciExampleComon::UTF8ToGBK((unsigned char*)psAsrRecogResult->psResultItemList[0].pszResult, &pucUTF8);
//			strMessage.AppendFormat("识别中间结果: %s", pucUTF8);
//			HciExampleComon::FreeConvertResult(pucUTF8);
//			pucUTF8 = NULL;
//		}
//    }
//    else
//    {
//        strMessage.AppendFormat( "*****无识别结果*****" );
//    }

//	dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
}

void SinoVoiceASRService::RecorderErr(
                               RECORDER_EVENT eRecorderEvent,
                               HCI_ERR_CODE eErrorCode,
                               void *pUsrParam)
{
//    CRecorder_ExampleDlg * dlg = (CRecorder_ExampleDlg*)pUsrParam;
//    CString strMessage = "";
//    strMessage.AppendFormat( "系统错误:%d", eErrorCode );

//	dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
}

void SinoVoiceASRService::RecorderRecordingCallback(
                                     unsigned char * pVoiceData,
                                     unsigned int uiVoiceLen,
                                     void * pUsrParam
                                     )
{
//	CRecorder_ExampleDlg * dlg = (CRecorder_ExampleDlg *)pUsrParam;
//	dlg->RecorderRecording(pVoiceData, uiVoiceLen);
}

