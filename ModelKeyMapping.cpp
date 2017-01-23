/*
 * @file  ModelKeyMapping.cpp
 * @note  Hikvision Digital Technology Co., Ltd. All Rights Reserved 
 * @brief model����keyֵӳ��
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
      * @brief ʹ��iSourceColumn��iSourceRole��Ӧ������ӳ�䵽��iTargetColumn��iTargetRole��Ӧ������
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
      * @brief ��ȡ����ӳ��
      * @param <IN> int iProxyColumn.
      * @param <IN> int iProxyRole .
      * @param <IN> int& iSourceColumn .
      * @param <IN> int& iSourceRole .
      * @return bool.
      */
    bool CModelKeyMapping::GetMapping(int iProxyColumn, int iProxyRole, int& iSourceColumn, int& iSourceRole) const
    {
        bool bRes = false;
        // ������ȡӳ��
        for (QList<MappingParam>::const_iterator it = m_listMappingParams.constBegin(); it != m_listMappingParams.constEnd(); ++it)
        {
            MappingParam struMappingParam = *it;
            // ���Ϊ-1�������е�ӳ�䣬������ƥ���Ƿ���ڲ�Ϊ-1��ӳ��
            if (struMappingParam.iProxyColumn == -1 && struMappingParam.iProxyRole == iProxyRole)
            {
                iSourceColumn = struMappingParam.iSourceColumn;
                iSourceRole = struMappingParam.iSourceRole;
                bRes = true;
            }
            // ��Ϊ-1��ʾֻƥ��������
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
