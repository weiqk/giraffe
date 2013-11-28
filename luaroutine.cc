/**
* @file luaroutine.cc
* @brief send to lua script
* @author ly
* @version 0.1.0
* @date 2013-11-28
*/
#include "luaroutine.h"
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>

using namespace std;
using namespace log4cxx;

LoggerPtr LuaRoutine::logger_(Logger::getLogger("luaroutine"));
 
const char load_file[] = "process.lua";

unsigned long count_pack = 0;
struct itimerval tick;  

/**
* @brief init luaroute thread
*/
void LuaRoutine::Init()
{
	InitZMQ();
	InitLua();
}

//void * LuaRoutine::ChildThreadFunc(void *arg)
//{
//	ChildThreadArg* item = (ChildThreadArg*)arg;
//	lua_State * state = item->lua_state;
//	while(true)
//	{
//		sleep(FLAGS_ROLL_OVERTIME);
//		lua_getglobal(state, "ObserverOvertime");
//		lua_pushinteger(state, item->market_id);
//
//		LOG4CXX_INFO(logger_, "ObserverOvertime send to lua");
//		if(lua_pcall(state, 1, 0, 0) != 0)
//		{
//			LOG4CXX_ERROR(logger_, lua_tostring(state, -1)); 
//			lua_pop(state,-1);
//			lua_close(state);
//		}
//		else
//		{
//			lua_pop(state, -1);	
//		}
//	}
//	return 0;	
//}

/**
* @brief init lua related
*/
void LuaRoutine::InitLua()
{
	lua_state_ = luaL_newstate();
	assert(NULL != lua_state_);
	luaL_openlibs(lua_state_);
	luaL_dofile(lua_state_ , load_file);
	
	lua_getglobal(lua_state_, "InitZMQ");
	//lua_pushinteger(lua_state_, lua_zmqpattern_);
	//lua_pushstring(lua_state_, lua_zmqsocketaction_.c_str());
	//lua_pushstring(lua_state_, lua_zmqsocketaddr_.c_str());
	//lua_pushlightuserdata(lua_state_, context_);

	if(lua_pcall(lua_state_, 0, 0, 0) != 0)
	{
		string s = lua_tostring(lua_state_,-1);
		lua_pop(lua_state_,-1);
		lua_close(lua_state_);
	}
	else
	{
		lua_pop(lua_state_, -1);	
	}

	vector<std::string> & did_template_ids = listening_item_.get_did_template_ids();
	for(vector<std::string>::iterator iter = did_template_ids.begin();iter != did_template_ids.end();iter++)
	{
		lua_getglobal(lua_state_, "init");
		lua_pushstring(lua_state_, (*iter).c_str());
		
		if(lua_pcall(lua_state_,1,0,0) != 0)
		{
			string s = lua_tostring(lua_state_,-1);
			lua_pop(lua_state_,-1);
			lua_close(lua_state_);
		}
		else
		{
			lua_pop(lua_state_, -1);
		}
	}
	//init ObserverOvertime
	//LOG4CXX_INFO(logger_, "before ObserverOvertime");
	//lua_getglobal(lua_state_, "ObserverOvertime");
	//lua_pushinteger(lua_state_, market_id_);

	//if(lua_pcall(lua_state_, 1, 0, 0) != 0)
	//{
	//	string s = lua_tostring(lua_state_,-1);
	//	lua_pop(lua_state_,-1);
	//	lua_close(lua_state_);
	//}
	//else
	//{
	//	lua_pop(lua_state_, -1);	
	//}
	
	//child_thread_arg_.lua_state = lua_state_;
	//child_thread_arg_.market_id = market_id_;	
	//int err;	
	//err = pthread_create(&tid_, NULL, ChildThreadFunc,(void*)&child_thread_arg_);
	//if (err!=0) 
	//{
	//	printf("exit\n");
	//	LOG4CXX_ERROR(logger_, "luarout create child thread error");
	//}
	//LOG4CXX_INFO(logger_, "after ObserverOvertime");
}

/**
* @brief init zmq
*/
void LuaRoutine::InitZMQ()
{
	assert(-1 != this->zmqitems_[0].zmqpattern);
	sock_ = new zmq::socket_t(*context_,this->zmqitems_[0].zmqpattern);
	//sock_->setsockopt(ZMQ_RCVHWM, &ZMQ_RCVHWM_SIZE, sizeof(ZMQ_RCVHWM_SIZE));
    if("bind" == this->zmqitems_[0].zmqsocketaction)
    {
        sock_->bind(this->zmqitems_[0].zmqsocketaddr.c_str());
    }
    else if("connect" == this->zmqitems_[0].zmqsocketaction)
    {
        sock_->connect(this->zmqitems_[0].zmqsocketaddr.c_str());
    }

	assert(-1 != this->zmqitems_[1].zmqpattern);
	sock_business_error_ = new zmq::socket_t(*context_, this->zmqitems_[1].zmqpattern);
	
    if("bind" == this->zmqitems_[1].zmqsocketaction)
    {
        sock_business_error_->bind(this->zmqitems_[1].zmqsocketaddr.c_str());
    }
    else if("connect" == this->zmqitems_[1].zmqsocketaction)
    {
        sock_business_error_->connect(this->zmqitems_[1].zmqsocketaddr.c_str());
    }
}

/**
* @brief return pointer of stk_static by stk_id
*
* @param stk_id
*
* @return 
*/
struct STK_STATIC * LuaRoutine::GetStkByID(int stk_id)
{
	assert(NULL != stk_static_);
	return stk_static_ + stk_id;	
}

//void LuaRoutine::DispatchToMonitor(int stk_id, const char *value)
//{
//	assert(NULL != value);
//	STK_STATIC * pstkstaticitem = GetStkByID(stk_id);
//	MonitorMsg *monitor_msg  = (MonitorMsg *)(monitor_mapping_file_->GetMapMsg());
//	time_t t;
//	t = time(&t);
//	dzh_time_t current_time(t);
//	monitor_msg->time = current_time;
//	strcpy(monitor_msg->error_type,"BUSINESS");
//	strcpy(monitor_msg->error_level,"WARNING");
//	strcpy(monitor_msg->stock_label, pstkstaticitem->m_strLabel);
//	strcpy(monitor_msg->error_info, value);
//}

/**
* @brief dispatch data to lua script
*
* @param pdcdata
* @param dc_type
* @param dc_general_intype
* @param stk_num
* @param did_template_id
*/
void LuaRoutine::DispatchToLua(unsigned char * pdcdata, int dc_type,int dc_general_intype, int stk_num, int did_template_id)
{
	assert(NULL != pdcdata);
	LOG4CXX_INFO(logger_, "dc_type:" << dc_type);
	//did
	if(DCT_DID == dc_type)
	{
		lua_getglobal(lua_state_, "process_did_type");
		lua_pushinteger(lua_state_, market_id_);
		lua_pushinteger(lua_state_, did_template_id);
		lua_pushinteger(lua_state_, stk_num);
		lua_pushlightuserdata(lua_state_, pdcdata);
		if(lua_pcall(lua_state_, 4, 1, 0) != 0)
		{
			//LOG4CXX_ERROR(logger_, lua_tostring(lua_state_,-1));
			lua_pop(lua_state_,-1);
			//lua_close(lua_state_);
		}
		else
		{
			if(lua_istable(lua_state_, -1))
			{
				lua_pushnil(lua_state_);
				while(0 != lua_next(lua_state_, -2))	
				{
					if(lua_isstring(lua_state_, -1))
					{
						LOG4CXX_INFO(logger_, "return string from lua:" << lua_tostring(lua_state_, -1));
						//zmq to business;	
						size_t len = lua_strlen(lua_state_, -1);
						zmq::message_t msg(len);
						memcpy(msg.data(), lua_tostring(lua_state_, -1), len);
						sock_business_error_->send(msg);
					}
					lua_remove(lua_state_, -1);
				}
			}
			lua_pop(lua_state_, -1);	
		}		
	}
	//static || dyna || shl2-mmpex || szl2-order-stat || szl2-order-five || szl2-trade-five
	else if (	DCT_STKSTATIC == dc_type 		|| \
				DCT_STKDYNA == dc_type 	 		|| \
				DCT_SHL2_MMPEx == dc_type 		|| \
				DCT_SZL2_ORDER_STAT == dc_type 	|| \
				DCT_SZL2_ORDER_FIVE == dc_type 	|| \
				DCT_SZL2_TRADE_FIVE == dc_type)
	{
		//working_lua
		//for(int i=0;i<stk_num;i++)
		//{
		//	count_pack += 1;
		//	//count_pack += stk_num*struct_size;
		//	lua_getglobal(lua_state_,"process");
		//	lua_pushinteger(lua_state_, dc_type);
		//	lua_pushlightuserdata(lua_state_,pdcdata+struct_size * i);
		//	//Sleep(50);
		//	if(lua_pcall(lua_state_,2,0,0) != 0)
		//	{
		//		string s = lua_tostring(lua_state_,-1);
		//		std::cout<<s<<endl;
		//		lua_pop(lua_state_,-1);
		//		lua_close(lua_state_);
		//	}
		//	else
		//	{
		//		//const char * lua_ret = lua_tostring(lua_state_,-1);
		//		//int stkid = lua_tonumber(lua_state_, -2);
		//		//if(NULL != lua_ret)
		//		//{
		//		//	//cout<<"lua stkid:"<<stkid<<"  lua_ret:"<<lua_ret<<endl;
		//		//	//DispatchToMonitor(stkid, lua_ret);
		//		//}
		//		lua_pop(lua_state_,-1);
		//	}
		//}
        lua_getglobal(lua_state_, "process_basic_type");
		lua_pushinteger(lua_state_, market_id_);
        lua_pushinteger(lua_state_, dc_type);
        lua_pushinteger(lua_state_, stk_num);
        lua_pushlightuserdata(lua_state_, pdcdata);
        if(lua_pcall(lua_state_,4,1,0) != 0)
        {
            string s = lua_tostring(lua_state_,-1);
			LOG4CXX_ERROR(logger_, s);
            lua_pop(lua_state_,-1);
            //lua_close(lua_state_);
        }
        else
        {
			if(lua_istable(lua_state_, -1))
			{
				lua_pushnil(lua_state_);
				while(0 != lua_next(lua_state_, -2))	
				{
					if(lua_isstring(lua_state_, -1))
					{
						LOG4CXX_INFO(logger_, "return string from lua:" << lua_tostring(lua_state_, -1));
						//zmq to business;	
						size_t len = lua_strlen(lua_state_, -1);
						zmq::message_t msg(len);
						memcpy(msg.data(), lua_tostring(lua_state_, -1), len);
						sock_business_error_->send(msg);
					}
					lua_remove(lua_state_, -1);
				}
			}
			lua_pop(lua_state_, -1);	
        }
	}
	else if(DCT_GENERAL == dc_type)
	{
		unsigned char *pdata = pdcdata + stk_num *sizeof(WORD);
		if(dc_general_intype == 5)
		{
			LOG4CXX_INFO(logger_, "intype is 5");
		}
		lua_getglobal(lua_state_, "process_general_type");
		lua_pushinteger(lua_state_, market_id_);
		lua_pushinteger(lua_state_, dc_general_intype);
		lua_pushinteger(lua_state_, stk_num);
		lua_pushlightuserdata(lua_state_, pdata);
		if(lua_pcall(lua_state_, 4, 1, 0) != 0)
		{
			//string s = lua_tostring(lua_state_,-1);
			//LOG4CXX_ERROR(logger_, s);
			lua_pop(lua_state_,-1);
			//lua_close(lua_state_);
		}
		else
		{
			if(lua_istable(lua_state_, -1))
			{
				lua_pushnil(lua_state_);
				while(0 != lua_next(lua_state_, -2))	
				{
					if(lua_isstring(lua_state_, -1))
					{
						LOG4CXX_INFO(logger_, "return string from lua:" << lua_tostring(lua_state_, -1));
						//zmq to business;	
						size_t len = lua_strlen(lua_state_, -1);
						zmq::message_t msg(len);
						memcpy(msg.data(), lua_tostring(lua_state_, -1), len);
						//sock_business_error_->send(msg);
					}
					lua_remove(lua_state_, -1);
				}
			}
			lua_pop(lua_state_, -1);	
		}
			
		//for(int i=0;i<stk_num;i++)
		//{
		//	lua_getglobal(lua_state_,"process_general");
		//	lua_pushinteger(lua_state_,dc_general_intype);
		//	lua_pushlightuserdata(lua_state_,pdata + struct_size * i);
		//	
		//	if(lua_pcall(lua_state_,2,0,0) != 0)
		//	{
		//		string s = lua_tostring(lua_state_,-1);
		//		LOG4CXX_ERROR(logger_, s);
		//		lua_pop(lua_state_,-1);
		//		lua_close(lua_state_);
		//	}
		//	else
		//	{
		//		lua_pop(lua_state_, -1);
		//	}
		//}
	}
	else if (DCT_SHL2_QUEUE)
	{
		/* code */
	}
	else
	{
			
	}
	free(pdcdata);
	pdcdata = NULL;
}

/**
* @brief thread running function
*/
void LuaRoutine::RunThreadFunc()
{
	//unsigned char * pdata = (unsigned char *)malloc(2000*sizeof(struct STK_STATIC));
	//memset(pdata, 0, 2000*sizeof(struct STK_STATIC));
	//struct STK_STATIC stk_static;
	//memset(stk_static.m_strLabel, 0, sizeof(stk_static.m_strLabel));
	//memcpy(stk_static.m_strLabel, "fdafsdf",5);
	//memset(stk_static.m_strName, 0, sizeof(stk_static.m_strName));
	//memcpy(stk_static.m_strName,"hhhhh",3);
	//stk_static.m_cType = DCT_STKSTATIC;
	//stk_static.m_nPriceDigit = '1';
	//stk_static.m_nVolUnit = 321;
	//stk_static.m_mFloatIssued = 134;
	//stk_static.m_mTotalIssued = 321;
	//stk_static.m_dwLastClose = 4324;
	//stk_static.m_dwAdvStop = 432;
	//stk_static.m_dwDecStop = 23423;
	//struct STK_STATIC *p = (struct STK_STATIC *)pdata; 

	//for(int i=0;i<2000;i++)
	//{
	//	memcpy(p+i,(unsigned char *)&stk_static,sizeof(stk_static));	
	//}
	
	zmq::message_t msg_rcv(sizeof(Lua_ZMQ_MSG_Item));
	zmq::pollitem_t items[] = {*sock_, 0, ZMQ_POLLIN, 0};
	while(true)
	{
			if(NULL == sock_)
			{
				LOG4CXX_INFO(logger_, "sock_ == NULL");
				continue;
			}
			int rc = zmq::poll(&items[0], 1, FLAGS_ROLL_OVERTIME * 1000);
			if(rc > 0)
			{
				if(items[0].revents & ZMQ_POLLIN)
				{
					LOG4CXX_INFO(logger_, "LuaRoutine:recv from uncomress");
					msg_rcv.rebuild();
					sock_->recv(&msg_rcv);
					Lua_ZMQ_MSG_Item *msg_item = (Lua_ZMQ_MSG_Item*)(msg_rcv.data());
					stk_static_ = msg_item->stk_static;
					market_id_ = msg_item->market_id;
					//free(msg_item->pdcdata);
					//msg_item->pdcdata = NULL;
					DispatchToLua(msg_item->pdcdata, msg_item->dc_type, msg_item->dc_general_intype, msg_item->stk_num, msg_item->did_template_id);
					//DispatchToLua(pdata, DCT_STKSTATIC, 0, 2000, sizeof(stk_static), 0);
				}
			}
			else if (rc == 0)//timeout
			{
				//sleep(FLAGS_ROLL_OVERTIME);
				lua_getglobal(lua_state_, "ObserverOvertime");
				if(market_id_ != 0)
				{
					lua_pushinteger(lua_state_, market_id_);
				}
				else
				{
					lua_pushinteger(lua_state_, 1);
				}

				LOG4CXX_INFO(logger_, "ObserverOvertime send to lua");
				if(lua_pcall(lua_state_, 1, 1, 0) != 0)
				{
					//LOG4CXX_ERROR(logger_, lua_tostring(lua_state_, -1)); 
					lua_pop(lua_state_,-1);
					//lua_close(lua_state_);
				}
				else
				{
					if(lua_istable(lua_state_, -1))
					{
						lua_pushnil(lua_state_);
						while(0 != lua_next(lua_state_, -2))	
						{
							if(lua_isstring(lua_state_, -1))
							{
								LOG4CXX_INFO(logger_, "return string from lua:" << lua_tostring(lua_state_, -1));
								//zmq to business;	
								size_t len = lua_strlen(lua_state_, -1);
								zmq::message_t msg(len);
								memcpy(msg.data(), lua_tostring(lua_state_, -1), len);
								sock_business_error_->send(msg);
							}
							lua_remove(lua_state_, -1);
						}
					}
					lua_pop(lua_state_, -1);	
				}
			}
			else
			{
				LOG4CXX_ERROR(logger_, "lua zmq poll fail");
			}
	}
}
