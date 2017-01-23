/*
 * @file  Combination_Model.h
 * @note  Hikvision Digital Technology Co., Ltd. All Rights Reserved 
 * @brief �������ʾ���ԭʼmodel�����ݣ�ʵ�������Լ��źŵ�ͬ��
 *    ����ͬ��ʵ�ֹ������£�
 *       ������̣�
 *       1.AboutToBeInserted��
 *       new MappingParam
 *       vsSources[] vsProxys[]
 *
 *       vsSources[0] vsProxys[0]
 *       2.Inserted��
 *       vsSources[0] vsProxys[-1, 0]
 *       beginInsertRows
 *       vsSources[0,1] vsProxys[0,1]
 *       endInsertRows
 *       
 *       ɾ�����̣�
 *       1.AboutToBeRemoved:
 *       vsSources[0,1]  vsProxys[0,1]
 *       beginRemoveRows
 *       vsSources[1] vsProxys[-1,0]
 *       endRemoveRows
 *       2.Removed:
 *       vsSources[0] vsProxys[0]
 * @author shengchangchun 
 * @date   2013-9-24
 * 
 * @note 
 * @note 
 */

#ifndef _GUITOOLKIT_COMBINATIONMODEL_H
#define _GUITOOLKIT_COMBINATIONMODEL_H

#include "iVMSGUIToolkitGlobal.h"
#include "Table/ModelKeyMapping.h"
#include <QAbstractItemModel>

namespace iVMSGUIToolkit
{
    class CCombination_ModelPrivate;

    class iVMS_GUI_EXPORT CCombination_Model : public QAbstractItemModel,
        public iVMSGUIToolkit::CModelKeyMapping
    {
        Q_OBJECT
    public:
        // ����
        CCombination_Model(QObject* pParent = NULL);
        ~CCombination_Model();

    public:
        // ������������(������������Դmodel�����иı䣬��������)
        void SetColumnCount(int iCounts);
        // ����root�ڵ�model(pRootModel���ܴ���������)
        // ֻ������һ��
        void SetRootModel(QAbstractItemModel* pRootModel);
        // ��ȡroot�ڵ�model
        QAbstractItemModel* GetRootModel() const;

        // ���pSourceModel�����CCombinationModel��
        // �Ե�һ���ڵ�����׷��(pSourceModel���ܴ���������)
        // 1.��Դmodel�ڴ���model����֮ǰ�����е���Ϣ���տ��Ի�ȡ�����������޷���ȡ������
        // 2.��Դmodel�ڴ���model����֮����Ϣ����
        bool AddModel(QAbstractItemModel* pSourceModel);
        // ��Դmodel���˳�����λ��iIndex����
        bool InsertModel(int iIndex, QAbstractItemModel* pSourceModel);
        // ��Դmodel���˳�����λ��iIndex����
        //bool InsertModels(int iIndex, QList<QAbstractItemModel*> listSourceModels);
        // ��pSourceModel�Ƴ�,�ᴥ��ɾ����Ϣ
        bool RemoveModel(QAbstractItemModel* pSourceModel);
        // ��pSourceModel����,�ᴥ��ɾ����Ϣ
        bool DeleteModel(QAbstractItemModel* pSourceModel);
        // �Ƴ�����Դmodel,�ᴥ��ɾ����Ϣ
        void RemoveAllModels();
        // �������е�Դmodel,�ᴥ��ɾ����Ϣ
        void DeleteAllModels();
        // ͨ������������ȡԴmodel
        QAbstractItemModel* GetModelByProxyIndex(const QModelIndex& miProxy) const;
        // �������˳��ֵ��ȡԴmodel
        QAbstractItemModel* GetModel(int iIndex) const;
        // ��ȡ������ӵ����model�е�model
        void GetAllModels(QVector<QAbstractItemModel*>& vsModels) const;
        // ��ȡ������ӵ�CCombination_Model�е�model����
        int GetModelCount() const;
        
    public:
        // ����������ת��Դ����
        QModelIndex MapToSource(const QModelIndex& miProxy) const;
        // ��Դ����ת�ɴ�������
        QModelIndex MapFromSource(const QModelIndex& miSource) const;

    public:
        // ��ȡ����
        virtual QModelIndex index(int iRow, int iColumn, const QModelIndex& miParent = QModelIndex()) const;
        // ��ȡ������
        virtual QModelIndex parent(const QModelIndex& miChild) const;
        // ��ȡ����
        virtual int rowCount(const QModelIndex& miParent = QModelIndex()) const;
        // ��ȡ����
        virtual int columnCount(const QModelIndex& miParent = QModelIndex()) const;
        // �Ƿ����ӽڵ�
        virtual bool hasChildren(const QModelIndex& miParent = QModelIndex()) const;
        // ��ȡ����
        virtual QVariant data(const QModelIndex& miIndex, int iRole = Qt::DisplayRole) const;
        // ��������
        virtual bool setData(const QModelIndex& miIndex, const QVariant& vtValue, int iRole = Qt::EditRole);
        // ��ȡ��������
        virtual QVariant headerData(int iSection, Qt::Orientation enumOrientation, int iRole = Qt::DisplayRole) const;
        // ���ñ�������
        virtual bool setHeaderData(int iSection, Qt::Orientation enumOrientation, const QVariant& vtValue, int iRole = Qt::EditRole);
        // ������
        virtual bool insertRows(int iRow, int iCount, const QModelIndex& miParent = QModelIndex());
        // ������
        virtual bool insertColumns(int iColumn, int iCount, const QModelIndex& miParent = QModelIndex());
        // ɾ����
        virtual bool removeRows(int iRow, int iCount, const QModelIndex& miParent = QModelIndex());
        // ɾ����
        virtual bool removeColumns(int iColumn, int iCount, const QModelIndex& miParent = QModelIndex());
        // ��ȡ�������
        virtual Qt::ItemFlags flags(const QModelIndex& miIndex) const;

    private:
        // ����root���źŲ�
        void ConnectRoot(QAbstractItemModel* pRootModel) const;
    private:
        QScopedPointer<CCombination_ModelPrivate> d_ptr;

    private:
        Q_DECLARE_PRIVATE(CCombination_Model)
        Q_DISABLE_COPY(CCombination_Model)

        // Դmodel
        Q_PRIVATE_SLOT(d_func(), void OnSourceDataChanged(const QModelIndex& miSourceTopLeft, const QModelIndex& miSourceBottomRight))
        Q_PRIVATE_SLOT(d_func(), void OnSourceAboutToBeReset())
        Q_PRIVATE_SLOT(d_func(), void OnSourceReset())
        Q_PRIVATE_SLOT(d_func(), void OnSourceModelDestroyed(QObject* pObject))
        Q_PRIVATE_SLOT(d_func(), void OnSourceLayoutAboutToBeChanged())
        Q_PRIVATE_SLOT(d_func(), void OnSourceLayoutChanged())
        Q_PRIVATE_SLOT(d_func(), void OnSourceRowsAboutToBeInserted(const QModelIndex& miSourceParent, int iStart, int iEnd))
        Q_PRIVATE_SLOT(d_func(), void OnSourceRowsInserted(const QModelIndex& miSourceParent, int iStart, int iEnd))
        Q_PRIVATE_SLOT(d_func(), void OnSourceRowsAboutToBeRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd))
        Q_PRIVATE_SLOT(d_func(), void OnSourceRowsRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd))
        Q_PRIVATE_SLOT(d_func(), void OnSourceColumnsAboutToBeInserted(const QModelIndex& miSourceParent, int iStart, int iEnd))
        Q_PRIVATE_SLOT(d_func(), void OnSourceColumnsInserted(const QModelIndex& miSourceParent, int iStart, int iEnd))
        Q_PRIVATE_SLOT(d_func(), void OnSourceColumnsAboutToBeRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd))
        Q_PRIVATE_SLOT(d_func(), void OnSourceColumnsRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd))
        Q_PRIVATE_SLOT(d_func(), void OnClearMapping())

        // root model
        Q_PRIVATE_SLOT(d_func(), void OnRootDataChanged(const QModelIndex& miSourceTopLeft, const QModelIndex& miSourceBottomRight))
        Q_PRIVATE_SLOT(d_func(), void OnRootHeaderDataChanged(Qt::Orientation enumOrientation, int iStart, int iEnd))
        Q_PRIVATE_SLOT(d_func(), void OnRootRowsAboutToBeInserted(const QModelIndex& miSourceParent, int iStart, int iEnd))
        Q_PRIVATE_SLOT(d_func(), void OnRootRowsInserted(const QModelIndex& miSourceParent, int iStart, int iEnd))
        Q_PRIVATE_SLOT(d_func(), void OnRootRowsAboutToBeRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd))
        Q_PRIVATE_SLOT(d_func(), void OnRootRowsRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd))
        Q_PRIVATE_SLOT(d_func(), void OnRootModelDestroyed(QObject* pObject))

    };
}

#endif // _GUITOOLKIT_COMBINATIONMODEL_H
