/*
 * @file  ModelKeyMapping.h
 * @note  Hikvision Digital Technology Co., Ltd. All Rights Reserved 
 * @brief model数据key值映射
 * 
 * @author shengchangchun 
 * @date   2013-9-25
 * 
 * @note 
 * @note 
 */

#ifndef _UITOOLKIT_MODELKEYMAPPING_H
#define _UITOOLKIT_MODELKEYMAPPING_H

#include "iVMSGUIToolkitGlobal.h"
#include <QList>

namespace iVMSGUIToolkit
{
    /**  类名：CModelKeyMapping
     *   说明 model数据key值映射
    **/
    class iVMS_GUI_EXPORT CModelKeyMapping
    {
        struct MappingParam
        {
            // 映射参数
            MappingParam(int iProxyColumn, int iProxyRole, int iSourceColumn, int iSourceRole)
                : iProxyColumn(iProxyColumn),
                iProxyRole(iProxyRole),
                iSourceColumn(iSourceColumn),
                iSourceRole(iSourceRole)
            {

            }

            int iProxyColumn;  // 代理列
            int iProxyRole;    // 代理key值
            int iSourceColumn; // 源数据列
            int iSourceRole;   // 源数据key值
        };

    public:
        // 添加role值映射 注:此为做了显示假象,不可做为获取数据依据
        // 使列iProxyColumn的iProxyRole的数据映射到列iSourceColumn的iSourceRole对应的数据
        // iProxyColumn数据从iSourceColumn获取
        // 如果iProxyColumn传入-1表示所有列都映射
        void AddMapping(int iProxyColumn, int iProxyRole, int iSourceColumn, int iSourceRole);
        
        // 获取映射关系
        bool GetMapping(int iProxyColumn, int iProxyRole, int& iSourceColumn, int& iSourceRole) const;
    private:
        QList<MappingParam> m_listMappingParams; // role值映射数据
    };

}

#endif // _UITOOLKIT_MODELKEYMAPPING_H
