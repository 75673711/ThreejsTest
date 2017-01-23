/*
 * @file  ModelKeyMapping.h
 * @note  Hikvision Digital Technology Co., Ltd. All Rights Reserved 
 * @brief model����keyֵӳ��
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
    /**  ������CModelKeyMapping
     *   ˵�� model����keyֵӳ��
    **/
    class iVMS_GUI_EXPORT CModelKeyMapping
    {
        struct MappingParam
        {
            // ӳ�����
            MappingParam(int iProxyColumn, int iProxyRole, int iSourceColumn, int iSourceRole)
                : iProxyColumn(iProxyColumn),
                iProxyRole(iProxyRole),
                iSourceColumn(iSourceColumn),
                iSourceRole(iSourceRole)
            {

            }

            int iProxyColumn;  // ������
            int iProxyRole;    // ����keyֵ
            int iSourceColumn; // Դ������
            int iSourceRole;   // Դ����keyֵ
        };

    public:
        // ���roleֵӳ�� ע:��Ϊ������ʾ����,������Ϊ��ȡ��������
        // ʹ��iProxyColumn��iProxyRole������ӳ�䵽��iSourceColumn��iSourceRole��Ӧ������
        // iProxyColumn���ݴ�iSourceColumn��ȡ
        // ���iProxyColumn����-1��ʾ�����ж�ӳ��
        void AddMapping(int iProxyColumn, int iProxyRole, int iSourceColumn, int iSourceRole);
        
        // ��ȡӳ���ϵ
        bool GetMapping(int iProxyColumn, int iProxyRole, int& iSourceColumn, int& iSourceRole) const;
    private:
        QList<MappingParam> m_listMappingParams; // roleֵӳ������
    };

}

#endif // _UITOOLKIT_MODELKEYMAPPING_H
