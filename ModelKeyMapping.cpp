/*
 * @file  ModelKeyMapping.cpp
 * @note  Hikvision Digital Technology Co., Ltd. All Rights Reserved 
 * @brief model数据key值映射
 * 
 * @author shengchangchun 
 * @date   2013-9-25
 * 
 * @note 
 * @note 
 */

#include "ModelKeyMapping.h"
#include "PublicControlDef.h"

namespace iVMSGUIToolkit
{
    /** @fn    void CModelKeyMapping::AddMapping(int iProxyColumn, int iProxyRole, int iSourceColumn, int iSourceRole)
      * @brief 使列iSourceColumn的iSourceRole对应的数据映射到列iTargetColumn的iTargetRole对应的数据
      * @param <IN> int iProxyColumn.
      * @param <IN> int iProxyRole.
      * @param <IN> int iSourceColumn.
      * @param <IN> int iSourceRole.
      * @return none.
      */
    void CModelKeyMapping::AddMapping(int iProxyColumn, int iProxyRole, int iSourceColumn, int iSourceRole)
    {
        m_listMappingParams.append(MappingParam(iProxyColumn, iProxyRole, iSourceColumn, iSourceRole));
    }

    /** @fn    bool CModelKeyMapping::GetMapping(int iProxyColumn, int iProxyRole, int& iSourceColumn, int& iSourceRole) const
      * @brief 获取数据映射
      * @param <IN> int iProxyColumn.
      * @param <IN> int iProxyRole .
      * @param <IN> int& iSourceColumn .
      * @param <IN> int& iSourceRole .
      * @return bool.
      */
    bool CModelKeyMapping::GetMapping(int iProxyColumn, int iProxyRole, int& iSourceColumn, int& iSourceRole) const
    {
        bool bRes = false;
        // 遍历获取映射
        for (QList<MappingParam>::const_iterator it = m_listMappingParams.constBegin(); it != m_listMappingParams.constEnd(); ++it)
        {
            MappingParam struMappingParam = *it;
            // 如果为-1代表所有的映射，但继续匹配是否存在不为-1的映射
            if (struMappingParam.iProxyColumn == -1 && struMappingParam.iProxyRole == iProxyRole)
            {
                iSourceColumn = struMappingParam.iSourceColumn;
                iSourceRole = struMappingParam.iSourceRole;
                bRes = true;
            }
            // 不为-1表示只匹配输入列
            else if (struMappingParam.iProxyColumn == iProxyColumn && struMappingParam.iProxyRole == iProxyRole)
            {
                iSourceColumn = struMappingParam.iSourceColumn;
                iSourceRole = struMappingParam.iSourceRole;
                return true;
            }
        }
        
        return bRes;
    }
}
