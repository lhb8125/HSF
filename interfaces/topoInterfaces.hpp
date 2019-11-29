/**
* @file: interfaces.hpp
* @author: Liu Hongbin
* @brief: fortran function interfaces
* @date:   2019-11-11 10:56:28
* @last Modified by:   lenovo
* @last Modified time: 2019-11-12 08:11:58
*/
#ifndef INTERFACES_HPP
#define INTERFACES_HPP

#include "utilities.hpp"
#include "mesh.hpp"
#include "region.hpp"

Array<Region> regs;
/**
* @brief fortran function interfaces
*/
/******************************************************************************
/**
* @brief 初始化程序，包括参数、网格读入，负载均衡，拓扑与通信对创建等
* @param [in] configFile 参数文件
*/
void init_(char* configFile);

/**
* @brief 获取网格单元数目
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param eleNum [out] 网格单元数目
*/
void getElementsNum_(int domain, int region, Label* eleNum);

/**
* @brief 获取所有网格面数目
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param faceNum [out] 所有网格面数目
*/
void getFacesNum_(int domain, int region, Label* faceNum);

/**
* @brief 获取所有内部网格面数目
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param innFaceNum [out] 所有内部网格面数目
*/
void getInnerFacesNum_(int domain, int region, Label* innFaceNum);

/**
* @brief 获取所有边界网格面数目
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param bndFaceNum [out] 所有边界网格面数目
*/
void getBndFacesNum_(int domain, int region, Label* bndFaceNum);

/**
* @brief 获取格点node数目
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param nodeNum [out] 格点node数目
*/
void getNodesNum_(int domain, int region, Label* nodeNum);

/******************************************************************************
/**
* @brief 获取网格单元与网格面的拓扑关系
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param ele2Face [out] 网格单元与网格面的映射关系，AoS格式存储 
* @param pos [out] AoS格式中结构体的起始位置
*/
void getEle2Face_(int domain, int region, Label* ele2Face，Label* pos);

/**
* @brief 获取网格单元之间的拓扑关系，只存储内部单元
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param ele2Ele [out] 网格单元之间的映射关系，AoS格式存储 
* @param pos [out] AoS格式中结构体的起始位置
*/
void getEle2Ele_(int domain, int region, Label* ele2Ele，Label* pos);

/**
* @brief 获取网格单元与格点的拓扑关系
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param ele2Node [out] 网格单元与格点的映射关系，AoS格式存储 
* @param pos [out] AoS格式中结构体的起始位置
*/
void getEle2Node_(int domain, int region, Label* ele2Node，Label* pos);

/**
* @brief 获取内部网格面与格点的拓扑关系
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param innFace2Node [out] 内部网格面与格点的映射关系，AoS格式存储 
* @param pos [out] AoS格式中结构体的起始位置
*/
void getInnFace2Node_(int domain, int region, Label* innFace2Node，Label* pos);

/**
* @brief 获取边界网格面与格点的拓扑关系
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param bndFace2Node [out] 边界网格面与格点的映射关系，AoS格式存储 
* @param pos [out] AoS格式中结构体的起始位置
*/
void getbndFace2Node_(int domain, int region, Label* bndFace2Node，Label* pos);

/**
* @brief 获取内部网格面与网格单元的拓扑关系
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param innFace2Ele [out] 内部网格面与网格单元的映射关系，AoS格式存储
* @param pos [out] AoS格式中结构体的起始位置
*/
void getInnFace2Ele_(int domain, int region, Label* innFace2Ele，Label* pos);

/**
* @brief 获取边界网格面与网格单元的拓扑关系
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param bndFace2Ele [out] 边界网格面与网格单元的映射关系，AoS格式存储 
* @param pos [out] AoS格式中结构体的起始位置
*/
void getbndFace2Ele_(int domain, int region, Label* bndFace2Ele，Label* pos);

/******************************************************************************
/**
* @brief 获取网格单元类型，以整型方式存储
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param eleType [out] 网格单元类型
*/
void getEleType_(int domain, int region, int* eleType);

/**
* @brief 获取格点坐标，以AoS方式存储
* @param domain [in] domain下标索引
* @param region [in] region下标索引
* @param coords [out] 格点坐标
*/
void getCoords_(int domain, int region, Scalar* coords);


#endif