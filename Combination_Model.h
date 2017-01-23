/*
 * @file  Combination_Model.h
 * @note  Hikvision Digital Technology Co., Ltd. All Rights Reserved 
 * @brief 可组合显示多个原始model的数据，实现数据以及信号的同步
 *    数据同步实现过程如下：
 *       插入过程：
 *       1.AboutToBeInserted：
 *       new MappingParam
 *       vsSources[] vsProxys[]
 *
 *       vsSources[0] vsProxys[0]
 *       2.Inserted：
 *       vsSources[0] vsProxys[-1, 0]
 *       beginInsertRows
 *       vsSources[0,1] vsProxys[0,1]
 *       endInsertRows
 *       
 *       删除过程：
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
        // 构造
        CCombination_Model(QObject* pParent = NULL);
        ~CCombination_Model();

    public:
        // 设置纵列数量(纵列数不依赖源model的纵列改变，需控制添加)
        void SetColumnCount(int iCounts);
        // 设置root节点model(pRootModel不能触发排序功能)
        // 只能设置一次
        void SetRootModel(QAbstractItemModel* pRootModel);
        // 获取root节点model
        QAbstractItemModel* GetRootModel() const;

        // 添加pSourceModel到组合CCombinationModel中
        // 以第一级节点往后追加(pSourceModel不能触发排序功能)
        // 1.若源model在代理model析构之前，所有的消息接收可以获取到索引，但无法获取到数据
        // 2.若源model在代理model析构之后，消息正常
        bool AddModel(QAbstractItemModel* pSourceModel);
        // 在源model添加顺序队列位置iIndex插入
        bool InsertModel(int iIndex, QAbstractItemModel* pSourceModel);
        // 在源model添加顺序队列位置iIndex插入
        //bool InsertModels(int iIndex, QList<QAbstractItemModel*> listSourceModels);
        // 将pSourceModel移除,会触发删除消息
        bool RemoveModel(QAbstractItemModel* pSourceModel);
        // 将pSourceModel销毁,会触发删除消息
        bool DeleteModel(QAbstractItemModel* pSourceModel);
        // 移除所有源model,会触发删除消息
        void RemoveAllModels();
        // 销毁所有的源model,会触发删除消息
        void DeleteAllModels();
        // 通过代理索引获取源model
        QAbstractItemModel* GetModelByProxyIndex(const QModelIndex& miProxy) const;
        // 根据添加顺序值获取源model
        QAbstractItemModel* GetModel(int iIndex) const;
        // 获取所有添加到组合model中的model
        void GetAllModels(QVector<QAbstractItemModel*>& vsModels) const;
        // 获取所有添加到CCombination_Model中的model个数
        int GetModelCount() const;
        
    public:
        // 将代理索引转成源索引
        QModelIndex MapToSource(const QModelIndex& miProxy) const;
        // 将源索引转成代理索引
        QModelIndex MapFromSource(const QModelIndex& miSource) const;

    public:
        // 获取索引
        virtual QModelIndex index(int iRow, int iColumn, const QModelIndex& miParent = QModelIndex()) const;
        // 获取父索引
        virtual QModelIndex parent(const QModelIndex& miChild) const;
        // 获取行数
        virtual int rowCount(const QModelIndex& miParent = QModelIndex()) const;
        // 获取列数
        virtual int columnCount(const QModelIndex& miParent = QModelIndex()) const;
        // 是否有子节点
        virtual bool hasChildren(const QModelIndex& miParent = QModelIndex()) const;
        // 获取数据
        virtual QVariant data(const QModelIndex& miIndex, int iRole = Qt::DisplayRole) const;
        // 设置数据
        virtual bool setData(const QModelIndex& miIndex, const QVariant& vtValue, int iRole = Qt::EditRole);
        // 获取标题数据
        virtual QVariant headerData(int iSection, Qt::Orientation enumOrientation, int iRole = Qt::DisplayRole) const;
        // 设置标题数据
        virtual bool setHeaderData(int iSection, Qt::Orientation enumOrientation, const QVariant& vtValue, int iRole = Qt::EditRole);
        // 插入行
        virtual bool insertRows(int iRow, int iCount, const QModelIndex& miParent = QModelIndex());
        // 插入列
        virtual bool insertColumns(int iColumn, int iCount, const QModelIndex& miParent = QModelIndex());
        // 删除行
        virtual bool removeRows(int iRow, int iCount, const QModelIndex& miParent = QModelIndex());
        // 删除列
        virtual bool removeColumns(int iColumn, int iCount, const QModelIndex& miParent = QModelIndex());
        // 获取标记属性
        virtual Qt::ItemFlags flags(const QModelIndex& miIndex) const;

    private:
        // 连接root的信号槽
        void ConnectRoot(QAbstractItemModel* pRootModel) const;
    private:
        QScopedPointer<CCombination_ModelPrivate> d_ptr;

    private:
        Q_DECLARE_PRIVATE(CCombination_Model)
        Q_DISABLE_COPY(CCombination_Model)

        // 源model
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
