/**
* @file:
* @author: Liu Hongbin
* @brief:
* @date:   2019-09-20 14:22:31
* @last Modified by:   lenovo
* @last Modified time: 2020-01-07 10:39:03
*/
#ifndef PARAMETER_HPP 
#define PARAMETER_HPP

#include <iostream>
#include <fstream>
#include <cstring>
#include <yaml-cpp/yaml.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include "cstdlib"
#include "utilities.h"
#include "hsfMacro.h"

#define Gauss 1
#define linear 2
#define steadyState 3

#define DIC 1
#define DILU 2

namespace HSF
{

/**
* @brief Parameter control module
*/
class ControlPara {
private: 
	char paraFile_[CHAR_DIM];
	YAML::Node config_;
public:
	/**
	* @brief Constructor with paramter file
	*/
	ControlPara(const Word paraFile) {
		strcpy(paraFile_, paraFile.c_str());
		config_ = YAML::LoadFile(paraFile);
	};
	ControlPara(YAML::Node &config)
	{
		config_ = config;
	};
	/**
	* @brief deconstructor
	*/
	~ControlPara() {};

	/**
	* @brief overide [] operator
	*/
	ControlPara& operator[](const Word str)
	{
		// printf("%s\n", str.c_str());
		if(config_[str].Type()==YAML::NodeType::Null
			|| config_[str].Type()==YAML::NodeType::Undefined)
		{
			printf("%s\n", str.c_str());
			Terminate("get Parameter","The string tree is not correct!\n");
		} else
		{
			config_ = config_[str];
			return *this;
		}
	}
	/**
	* @brief read paramter
	*/
	template<typename T>
	void read(T& res)
	{
		if(config_.Type()!=YAML::NodeType::Scalar)
			Terminate("read","The node is not a leaf node");
		res = config_.as<T>();
		config_ = YAML::LoadFile(paraFile_);
	}
	/**
	* @brief transform paramter to result
	*/
	template<typename T>
	T to()
	{
		if(config_.Type()!=YAML::NodeType::Scalar)
			Terminate("to","The node is not a leaf node");
		T res = config_.as<T>();
		config_ = YAML::LoadFile(paraFile_);
		return res;
	}
	/**
	* @brief get the size of a sequence node
	*/
	label32 size()
	{
		if(config_.Type()!=YAML::NodeType::Sequence)
			Terminate("get Size","The node is not a sequence node");
		config_ = YAML::LoadFile(paraFile_);
		return config_.size();
	}

	void getNodeType(YAML::Node node) {
		switch (node.Type()) {
		  case YAML::NodeType::Null: printf("Null\n"); break;
		  case YAML::NodeType::Scalar: printf("Scalar\n"); break;
		  case YAML::NodeType::Sequence: printf("Sequence\n"); break;
		  case YAML::NodeType::Map: printf("Map\n"); break;
		  case YAML::NodeType::Undefined: printf("Undefined\n"); break;
		  default: printf("unknown\n"); break;
		}
	}

};
/**
* @brief Control parameter
*/
class Parameter {
  private:
	char paraFile_[CHAR_DIM];
	// Array<paraDomain> domain_;
  public:
	/**
	* @brief default constructor
	*/
	Parameter(const char* paraFile) {
		// strncpy(paraFile_, paraFile, length);
		strcpy(paraFile_, paraFile);
	};
	Parameter() {};
	/**
	* @brief deconstructor
	*/
	~Parameter() {};
	/**
	* @brief set the parameter file
	*/

	void setParaFile(const char* paraFile) {
		// strncpy(paraFile_, paraFile, length);
		strcpy(paraFile_, paraFile);
		// printf("%s, %s, %d, %d\n", paraFile_, paraFile, strlen(paraFile), length);
		// paraFile_[strlen(paraFile)]='\0';
	}
	// /**
	// * @brief read parameter through yaml-cpp
	// */
	// void readPara(const char* paraFile);

	/**
	* @brief get the parameter in config file
	* @param[in] nPara the count of parameters
	* @param[out] resVal return value
	* @param[in] ... other parameter
	* @tparam T char, int, float
	*/
	// void getPara(const int* nPara, void* resVal, const char* type, ...);
	template<typename T>
	void getPara(void* resVal, int nPara, ...);

	template<typename T>
	void getPara(void* resVal, char** strList, const int strLen);

	/**
	* @brief get the parameter in config file
	* @param[in] nPara the count of parameters
	* @param[out] resVal return value
	* @param[in] ... other parameter
	* @tparam T char, int, float
	*/
	// void getPara(const int* nPara, void* resVal, const char* type, ...);
	template<typename T>
	void getPara(Array<T*>& resVal, int nPara, ...);

	// void printPara();

	// paraDomain& getDomain(char* name);
};

template<typename T>
void Parameter::getPara(void* resVal, char** strList, const int strLen)
{
	Word configFile = paraFile_;
	YAML::Node config = YAML::LoadFile(configFile);
	if(!config) 
	{
		resVal = NULL;
		printf("YAML file: %s\n", paraFile_);
		Terminate("loadYAMLFile", "please check the parameter string");
	}

	// printf("%d,%s\n", strLen,strList[0]);
	for (int i = 0; i < strLen; ++i)
	{
// printf("%s\n", strList[i]);
		config = config[strList[i]];
		if(!config) 
		{
			resVal = NULL;
			Terminate("getPara", "please check the parameter string");
		}
	}
	Word res = config.as<Word>();
// printf("%s\n", res.c_str());
	// std::cout<<config.as<Word>()<<std::endl;
	if(typeid(T)==typeid(int))
	{
		int* tmp = (int*)resVal;
		tmp[0] = std::stoi(res);
	}else if(typeid(T)==typeid(char))
	{
		char* tmp = (char*)resVal;
		strcpy(tmp, res.c_str());
	}else if(typeid(T)==typeid(float))
	{
		scalar* tmp = (scalar*)resVal;
		tmp[0] = (scalar)std::stof(res);
	}else 
	{
		Terminate("reading parameters", "the type must be the basic type");
	}	
}

template<typename T>
void Parameter::getPara(void* resVal, int nPara, ...)
{
	Word configFile = paraFile_;
// printf("%s\n", configFile);
	YAML::Node config = YAML::LoadFile(configFile);
	if(!config) 
	{
		resVal = NULL;
		printf("YAML file: %s\n", paraFile_);
		Terminate("loadYAMLFile", "please check the parameter string");
	}
	va_list args;
	va_start(args, nPara);
	// printf("parameter num: %d, return type: %s\n", *nPara, type);

	char* para;
	for (int i = 0; i < nPara; ++i)
	{
		// printf("%s, \n", va_arg(args, char*));
		para = va_arg(args, char*);
		// printf("%s\n", para);
		config = config[para];
		if(!config) 
		{
			resVal = NULL;
			Terminate("getPara", "please check the parameter string");
		}
		// printf("%d\n", config.size());
		// paras.push_back(Word(para));
	}

	va_end(args);

	Word res = config.as<Word>();
	// std::cout<<config.as<Word>()<<std::endl;
	if(typeid(T)==typeid(int))
	{
		int* tmp = (int*)resVal;
		tmp[0] = std::stoi(res);
	}else if(typeid(T)==typeid(char))
	{
		char* tmp = (char*)resVal;
		strcpy(tmp, res.c_str());
	}else if(typeid(T)==typeid(float))
	{
		float* tmp = (float*)resVal;
		tmp[0] = std::stof(res);
	}else 
	{
		Terminate("reading parameters", "the type must be the basic type");
	}	
}


template<typename T>
void Parameter::getPara(Array<T*>& resVal, int nPara, ...)
{
	Word configFile = paraFile_;
	YAML::Node config = YAML::LoadFile(configFile);
	if(!config) 
	{
		printf("YAML file: %s\n", paraFile_);
		Terminate("loadYAMLFile", "please check the parameter string");
	}
	va_list args;
	va_start(args, nPara);
	// printf("parameter num: %d, return type: %s\n", *nPara, type);

	char* para;
	for (int i = 0; i < nPara; ++i)
	{
		// printf("%s, \n", va_arg(args, char*));
		para = va_arg(args, char*);
		// printf("%s\n", para);
		config = config[para];
		if(!config) 
		{
			Terminate("getPara", "please check the parameter string");
		}
		// printf("%d\n", config.size());
		// paras.push_back(Word(para));
	}

	va_end(args);

	int nRes = config.size();
	// 如果该节点为标量，则重设数组维度
	if(nRes==0) nRes=1;
	
	resVal.resize(nRes);

	for (int i = 0; i < nRes; ++i)
	{
		Word res;
		if(nRes>1) res = config[i].as<Word>();
		else res = config.as<Word>();

		// std::cout<<res<<std::endl;
		if(typeid(T)==typeid(int))
		{
			int* tmp = (int*)resVal[i];
			tmp[0] = std::stoi(res);
		}else if(typeid(T)==typeid(char))
		{
			char* tmp = (char*)resVal[i];
			strcpy(tmp, res.c_str());
		}else if(typeid(T)==typeid(float))
		{
			float* tmp = (float*)resVal[i];
			tmp[0] = std::stof(res);
		}else 
		{
			Terminate("reading parameters", "the type must be the basic type");
		}			
	}	
}

} // end namespace HSF

#endif
