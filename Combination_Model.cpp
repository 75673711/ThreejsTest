/*
 * @file  Combination_Model.cpp
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

#include "Combination_Model.h"
#include "PublicControlDef.h"
#include "PublicControlFunc.h"
#include <QStandardItemModel>
#include <QDebug>

namespace iVMSGUIToolkit
{
    class CCombination_ModelPrivate : public QObject
    {
        class CCombination_Model* q_ptr;
        Q_DECLARE_PUBLIC(CCombination_Model)// 在CCombinationModelPrivate中通过q_func()可以获得CCombinationModel的指针q_ptr

    public:
        CCombination_ModelPrivate(QObject* pParent = NULL)
            : QObject(pParent),
            m_pRootModel(NULL),
            m_pDefaultRootModel(NULL),
            m_pRootMappingParam(NULL),
            m_bRemRootModel(false)
        {

        }

        ~CCombination_ModelPrivate()
        {
        }
    public:
        struct MappingParam 
        {
        public:
            MappingParam()
                : pSourceModel(NULL),
                iProxyRowBegin(0)
            {
            }

            ~MappingParam()
            {
            }
        public:
            int ProxyRowToSourceRow(int iProxyRow) const
            {
                int iSourceRow = iProxyRow - iProxyRowBegin;
                if (iSourceRow < 0 || iSourceRow >= vsSourceRows.size())
                {
                    return -1;
                }

                return vsSourceRows.at(iSourceRow);
            }

            int ProxyColumnToSourceColumn(int iProxyColumn) const
            {
                if (iProxyColumn < 0 || iProxyColumn >= vsSourceColumns.size())
                {
                    return -1;
                }

                return vsSourceColumns.at(iProxyColumn);
            }

        public:
            QAbstractItemModel* pSourceModel; // 源model
            QVector<int> vsSourceRows;    // 源行
            QVector<int> vsSourceColumns; // 源列
            int iProxyRowBegin;           // 记录代理model的起始位置
            QVector<int> vsProxyRows;     // 源行的vector索引
            QVector<int> vsProxyColumns;  // 源列的vector索引
            QVector<QModelIndex> vsMappedChildrens;
            QHash<MappingParam*, QModelIndex>::const_iterator iterMap; // 记录当前结构体数据的迭代器
        };

        struct ModelMappingParam 
        {
        public:
            ModelMappingParam()
                : pRemModel(NULL)
            {

            }

        public:
            // 建立数据映射关系
            void UpdateMapping()
            {
                vsSources.clear();
                vsProxys.clear();
                for (int i = 0, iSize = vsSourceModels.size(); i < iSize; i++)
                {
                    vsSources.append(i);
                    vsProxys.append(i);
                }
            }
        public:
            QVector<int> vsSources;   // 源
            QVector<int> vsProxys;    // 代理
            QVector<QAbstractItemModel*> vsSourceModels; // 保存添加到组合CCombinationModel中的models
            QAbstractItemModel* pRemModel; // 如果model由外部删除，记录
        };

    public:
        mutable ModelMappingParam m_struModelMappingParam;
        mutable MappingParam* m_pRootMappingParam;
    public:
        MappingParam* CreateRootMapping() const
        {
            if (m_bRemRootModel)
            {
                return m_pRootMappingParam;
            }

            if (m_pRootMappingParam == NULL)
            {
                m_pRootMappingParam = new MappingParam;
                if (NULL != GetRootModel() && GetRootModel()->hasChildren())
                {
                    m_pRootMappingParam->vsSourceRows.append(0);
                    m_pRootMappingParam->vsProxyRows.append(0);
                }

                m_pRootMappingParam->pSourceModel = GetRootModel();
            }

            return m_pRootMappingParam;
        }
    public:
        int SourceRowToProxyRow(QVector<int> vsSourceRows, int iSourceRow, QVector<int> vsProxyRows, int iProxyRowBegin) const
        {
            if (!vsSourceRows.contains(iSourceRow))
            {
                return -1;
            }

            for (QVector<int>::const_iterator it = vsProxyRows.constBegin(); it != vsProxyRows.constEnd(); ++it)
            {
                int iProxyRow = *it;
                if (iProxyRow < 0 || iProxyRow >= vsSourceRows.size())
                {
                    continue;
                }

                if (vsSourceRows.at(iProxyRow) == iSourceRow)
                {
                    return iProxyRow + iProxyRowBegin;
                }
            }

            return -1;
        }

        int SourceColumnToProxyColumn(QVector<int> vsSourceColumns, int iSourceColumn, QVector<int> vsProxyColumns) const
        {
            if (!vsSourceColumns.contains(iSourceColumn))
            {
                return -1;
            }

            for (QVector<int>::const_iterator it = vsProxyColumns.constBegin(); it != vsProxyColumns.constEnd(); ++it)
            {
                int iProxyCol = *it;
                if (iProxyCol < 0 || iProxyCol >= vsSourceColumns.size())
                {
                    continue;
                }

                if (vsSourceColumns.at(iProxyCol) == iSourceColumn)
                {
                    return iProxyCol;
                }
            }

            return -1;
        }

    public:
        // 参数2为源model存在子节点的索引，不保存root model对应的节点
        // 参数1为参数2对于的节点数据
        mutable QHash<MappingParam*, QModelIndex> m_hashSourceIndexMappings;

    private:
        QAbstractItemModel* m_pRootModel; // root model
        bool m_bRemRootModel;
        mutable QAbstractItemModel* m_pDefaultRootModel;

    public:
        // 获取root model
        QAbstractItemModel* GetRootModel() const;
        // 通过索引获取源model
        QAbstractItemModel* GetSourceModelByProxyIndex(const QModelIndex& miProxy) const;
        // 通过组合的第一级节点的行号获取源
        QAbstractItemModel* GetSourceModelByProxyRow(int iProxyRow) const;
        
        // 获取源model的iSourceRow在组合model中的位置(只能是第一节点) 参数指pSourceModel的行
        int GetSourceRowToProxyRow(const QAbstractItemModel* pSourceModel, int iSourceRow) const;
        // 通过model位置获取行号
        void GetSourceModelToProxyRow(int iSourceModelIndex, int& iProxyStart, int& iProxyEnd) const;
    public:
        // 更新下所有的root节点(当添加或删除时)
        void UpdateAllRootMapping(int iModelIndex);
        // 更新pSourceModel的节点关联
        void UpdateModelMapping(QAbstractItemModel* pSourceModel, const QModelIndex& miSourceParent = QModelIndex());
        // 更新跟节点的映射
        void UpdateRootMappingProxyRowBegin();

    public:
        // 是否为有效的跟节点
        bool IsValidRootIndex(const QModelIndex& miProxy) const
        {
            QAbstractItemModel* pRootModel = GetRootModel();
            return ((pRootModel != NULL) && (pRootModel->hasChildren()) && (miProxy.isValid() && !miProxy.parent().isValid()));
        }

        // 是否为有效的跟的父节点
        bool IsValidRootParentIndex(const QModelIndex& miProxyParent) const
        {
            QAbstractItemModel* pRootModel = GetRootModel();
            return ((pRootModel != NULL) && (pRootModel->hasChildren()) && !miProxyParent.isValid());
        }

    public:
        // 创建映射关系
        bool CreateMapping(const QModelIndex& miSourceParent, const QAbstractItemModel* pSourceModel, QHash<MappingParam*, QModelIndex>::const_iterator& it) const;
        // 将代理索引转成源索引
        QModelIndex ProxyToSource(const QModelIndex& miProxy, bool bFilterRoot = false) const;
        // 将源索引转成代理索引
        QModelIndex SourceToProxy(const QModelIndex& miSource) const;
        // 删除映射
        void RemoveFromMapping(const QModelIndex& miSourceParent, const QAbstractItemModel* pSourceModel);
        // 取出映射
        MappingParam* TakeSourceIndexToIterator(const QModelIndex& miSourceParent, const QAbstractItemModel* pSourceModel)
        {
            QHash<MappingParam*, QModelIndex>::iterator it = m_hashSourceIndexMappings.begin();
            while(it != m_hashSourceIndexMappings.end())
            {
                if (it.value() == miSourceParent && it.key()->pSourceModel == pSourceModel)
                {
                    MappingParam* pMappingParam = it.key();
                    m_hashSourceIndexMappings.erase(it);
                    return pMappingParam;
                }

                ++it;
            }

            return NULL;
        }

        bool SourceIndexToIterator(const QModelIndex& miSourceParent, int iProxyRow, QHash<MappingParam*, QModelIndex>::const_iterator& it) const
        {
            it = m_hashSourceIndexMappings.constBegin();
            while(it != m_hashSourceIndexMappings.constEnd())
            {
                const MappingParam* pMappingParam = it.key();

                if (it.value() == miSourceParent && pMappingParam->vsSourceRows.contains(iProxyRow - pMappingParam->iProxyRowBegin))
                {
                    return true;
                }

                ++it;
            }

            return false;
        }

        bool SourceIndexToIterator(const QModelIndex& miSourceParent, const QAbstractItemModel* pSourceModel, QHash<MappingParam*, QModelIndex>::const_iterator& it) const
        {
            it = m_hashSourceIndexMappings.constBegin();
            while(it != m_hashSourceIndexMappings.constEnd())
            {
                if (it.value() == miSourceParent && it.key()->pSourceModel == pSourceModel)
                {
                    return true;
                }

                ++it;
            }

            return false;
        }

        bool ProxyIndexToIterator(const QModelIndex& miProxy, QHash<MappingParam*, QModelIndex>::const_iterator& it) const
        {
            if (!miProxy.isValid())
            {
                return false;
            }

            const void* pPointer = miProxy.internalPointer();
            if (NULL == pPointer)
            {
                //Q_ASSERT(pPointer);
                return false;
            }

            it = static_cast<const MappingParam*>(pPointer)->iterMap;

            return true;
        }

        inline QModelIndex CreateIndex(int iRow, int iColumn, QHash<MappingParam*, QModelIndex>::const_iterator it) const
        {
            return q_func()->createIndex(iRow, iColumn, it.key());
        }

        void OnSourceDataChanged(const QModelIndex& miSourceTopLeft, const QModelIndex& miSourceBottomRight);

        void OnSourceAboutToBeReset();
        void OnSourceReset();

        void OnSourceLayoutAboutToBeChanged();
        void OnSourceLayoutChanged();

        void OnSourceRowsAboutToBeInserted(const QModelIndex& miSourceParent, int iStart, int iEnd);
        void OnSourceRowsInserted(const QModelIndex& miSourceParent, int iStart, int iEnd);
        void OnSourceRowsAboutToBeRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd);
        void OnSourceRowsRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd);
        void OnSourceColumnsAboutToBeInserted(const QModelIndex& miSourceParent, int iStart, int iEnd);
        void OnSourceColumnsInserted(const QModelIndex& miSourceParent, int iStart, int eiEndnd);
        void OnSourceColumnsAboutToBeRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd);
        void OnSourceColumnsRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd);

        void OnClearMapping();
        void Clear();

        void BuildSourceToProxyMapping(const QVector<int>& vsSources, QVector<int>& vsProxys) const;
        void SourceModelAboutToBeInserted(const QAbstractItemModel* pSourceModel, const QModelIndex& miSourceParent);
        void SourceModelInserted(const QAbstractItemModel* pSourceModel, const QModelIndex& miSourceParent,
            int iStart, int iEnd,  Qt::Orientation enumOrient);
        void SourceModelAboutToBeRemoved(QAbstractItemModel* pSouceModel, const QModelIndex& miSourceParent,
            int iStart, int iEnd,  Qt::Orientation enumOrient, bool bRemModel = false);
        void SourceModelRemoved(QAbstractItemModel* pSouceModel, const QModelIndex& miSourceParent,
            int iStart, int iEnd,  Qt::Orientation enumOrient);

        void UpdateChildrenMapping(const QAbstractItemModel* pSourceModel, const QModelIndex& miSourceParent, MappingParam *parent_mapping,
            Qt::Orientation enumOrient, int iStart, int iEnd, int delta_item_count, bool remove);

        void OnSourceModelDestroyed(QObject* pObject);

        inline bool IsIndexValid(const QModelIndex& miIndex) const 
        {
            return (miIndex.row() >= 0) && (miIndex.column() >= 0) && (miIndex.model() == q_func());
        }

    public:
        void OnRootDataChanged(const QModelIndex& miSourceTopLeft, const QModelIndex& miSourceBottomRight);
        void OnRootHeaderDataChanged( Qt::Orientation enumOrientation, int iStart, int iEnd);

        void OnRootRowsAboutToBeInserted(const QModelIndex& miSourceParent, int iStart, int iEnd);
        void OnRootRowsInserted(const QModelIndex& miSourceParent, int iStart, int iEnd);
        void OnRootRowsAboutToBeRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd);
        void OnRootRowsRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd);

        void OnRootModelDestroyed(QObject* pObject);
    };

    typedef QHash<CCombination_ModelPrivate::MappingParam*, QModelIndex> IndexMap;

    void CCombination_ModelPrivate::OnSourceModelDestroyed(QObject* pObject)
    {
        // 由于model析构之后就不能调用其所有接口，但必须发送其信号
        QVector<QAbstractItemModel*>& vsSourceModels = m_struModelMappingParam.vsSourceModels;
        for (QVector<QAbstractItemModel*>::Iterator it = vsSourceModels.begin(); it != vsSourceModels.end(); ++it)
        {
            QAbstractItemModel* pSourceModel = *it;
            if (pSourceModel == pObject)
            {
                m_struModelMappingParam.pRemModel = pSourceModel;
                q_func()->RemoveModel(pSourceModel);
                qWarning("---CCombination_ModelPrivate::OnSourceModelDestroyed--");
                m_struModelMappingParam.pRemModel = NULL;

                break;
            }
        }
    }

    void CCombination_ModelPrivate::RemoveFromMapping(const QModelIndex& miSourceParent, const QAbstractItemModel* pSourceModel)
    {
        if (MappingParam* pMappingParam = TakeSourceIndexToIterator(miSourceParent, pSourceModel)) 
        {
            for (int i = 0; i < pMappingParam->vsMappedChildrens.size(); ++i)
            {
                RemoveFromMapping(pMappingParam->vsMappedChildrens.at(i), pSourceModel);
            }

            delete pMappingParam;
        }
    }

    void CCombination_ModelPrivate::OnClearMapping()
    {
        // 清除源model的数据关联信息
        for (IndexMap::Iterator it = m_hashSourceIndexMappings.begin(); it != m_hashSourceIndexMappings.end(); )
        {
            MappingParam* pMapping = it.key();
            it = m_hashSourceIndexMappings.erase(it);
            delete pMapping;
        }

        m_hashSourceIndexMappings.clear();

        // 如果代理model析构，不做销毁操作，防止触发删除
        QVector<QAbstractItemModel*>& vsSourceModels = m_struModelMappingParam.vsSourceModels;
        for (QVector<QAbstractItemModel*>::Iterator it = vsSourceModels.begin(); it != vsSourceModels.end(); ++it)
        {
            disconnect(*it, SIGNAL(destroyed(QObject*)), q_func(), 0);
        }
        vsSourceModels.clear();

        if (m_pRootMappingParam != NULL)
        {
            delete m_pRootMappingParam;
            m_pRootMappingParam = NULL;
        }
    }

    void CCombination_ModelPrivate::Clear()
    {
        OnClearMapping();
        if (NULL != m_pRootModel)
        {
            disconnect(m_pRootModel, SIGNAL(destroyed(QObject*)), q_func(), 0);
        }

        if (NULL != m_pDefaultRootModel)
        {
            disconnect(m_pDefaultRootModel, SIGNAL(destroyed(QObject*)), q_func(), 0);
        }
    }

    // 使用此接口需先判断是否存在，除非确认第一次创建
    bool CCombination_ModelPrivate::CreateMapping(const QModelIndex& miSourceParent, 
        const QAbstractItemModel* pSourceModel, QHash<MappingParam*, QModelIndex>::const_iterator& it) const
    {
        Q_Q(const CCombination_Model);
        if (NULL == pSourceModel)
        {
            return false;
        }

        if (pSourceModel == m_struModelMappingParam.pRemModel)
        {
            return false;
        }

        MappingParam* pMappingParam = new MappingParam;
        pMappingParam->pSourceModel = const_cast<QAbstractItemModel*>(pSourceModel);

        // 记录源model行
        int iSourceRows = pSourceModel->rowCount(miSourceParent);
        pMappingParam->vsSourceRows.reserve(iSourceRows);
        for (int i = 0; i < iSourceRows; ++i) 
        {
            pMappingParam->vsSourceRows.append(i);
        }

        // 记录源model列
        int iSourceCols = pSourceModel->columnCount(miSourceParent);
        pMappingParam->vsSourceColumns.reserve(iSourceCols);
        for (int i = 0; i < iSourceCols; ++i) 
        {
            pMappingParam->vsSourceColumns.append(i);
        }

        if (pMappingParam->vsSourceRows.size() > 0)
        {
            Q_ASSERT(pMappingParam->vsSourceRows.size() != 0);
        }

        int iRow = 0;
        if (pSourceModel != GetRootModel() && !miSourceParent.isValid()) // 如果无效,说明是root节点
        {
            iRow = GetSourceRowToProxyRow(pSourceModel, 0);
        }

        pMappingParam->iProxyRowBegin = iRow;
        // 记录代理model数据
        pMappingParam->vsProxyRows.resize(iSourceRows);
        BuildSourceToProxyMapping(pMappingParam->vsSourceRows, pMappingParam->vsProxyRows);
        pMappingParam->vsProxyColumns.resize(iSourceCols);
        BuildSourceToProxyMapping(pMappingParam->vsSourceColumns, pMappingParam->vsProxyColumns);

        it = IndexMap::const_iterator(m_hashSourceIndexMappings.insert(pMappingParam, miSourceParent));
        pMappingParam->iterMap = it;

        if (miSourceParent.isValid()) 
        {
            QModelIndex miSourceGrandParent = miSourceParent.parent();
            IndexMap::const_iterator it2;
            if (SourceIndexToIterator(miSourceGrandParent, pSourceModel, it2) || CreateMapping(miSourceGrandParent, pSourceModel, it2))
            {
                MappingParam* pMappingParam2 = it2.key();
                if (NULL != pMappingParam2)
                {
                    if (!pMappingParam2->vsMappedChildrens.contains(miSourceParent))
                    {
                        pMappingParam2->vsMappedChildrens.append(miSourceParent);
                    }
                }
            }
        }

        return true;
    }

    // 获取root model
    QAbstractItemModel* CCombination_ModelPrivate::GetRootModel() const
    {
        Q_Q(const CCombination_Model);
        if (NULL != m_pRootModel && !m_bRemRootModel)
        {
            return m_pRootModel;
        }

        if (NULL == m_pDefaultRootModel)
        {
            m_pDefaultRootModel = new QStandardItemModel(const_cast<CCombination_Model*>(q));
            q->ConnectRoot(m_pDefaultRootModel);
        }

        return m_pDefaultRootModel;
    }

    // 获取CCombinationModel中的索引miCur在被添加model
    QAbstractItemModel* CCombination_ModelPrivate::GetSourceModelByProxyIndex(const QModelIndex& miProxy) const
    {
        Q_Q(const CCombination_Model);

        QModelIndex miSourceParent = ProxyToSource(miProxy.parent(), true);
        IndexMap::const_iterator it;
        if (SourceIndexToIterator(miSourceParent, miProxy.row(), it))
        {
            CCombination_ModelPrivate::MappingParam* pMappingParam = it.key();
            return pMappingParam->pSourceModel;
        }

        return NULL;
    }

    QAbstractItemModel* CCombination_ModelPrivate::GetSourceModelByProxyRow(int iProxyRow) const
    {
        QVector<int> vsSources = m_struModelMappingParam.vsSources;   // 源
        QVector<int> vsProxys = m_struModelMappingParam.vsProxys;     // 代理
        QVector<QAbstractItemModel*> vsSourceModels = m_struModelMappingParam.vsSourceModels;
        for (QVector<int>::const_iterator it = vsProxys.constBegin(); it != vsProxys.constEnd(); ++it)
        {
            // 先找到源model
            int iProxy = *it;
            if (iProxy < 0 || iProxy >= vsSources.size())
            {
                continue;
            }

            int iSource = vsSources.at(iProxy);
            if (iSource < 0 || iSource >= vsSourceModels.size())
            {
                return NULL;
            }

            QAbstractItemModel* pSourceModel = vsSourceModels[iSource];

            // 遍历找到映射数据
            QHash<MappingParam*, QModelIndex>::const_iterator itMaps;
            if (SourceIndexToIterator(QModelIndex(), pSourceModel, itMaps))
            {
                int iSourceRows = itMaps.key()->vsSourceRows.size();
                iProxyRow = iProxyRow - iSourceRows;
                if (iProxyRow < 0)
                {
                    return pSourceModel;
                }
            }
        }

        return NULL;
    }

    // 获取源model的row在组合model中的位置(只能是第一节点)
    int CCombination_ModelPrivate::GetSourceRowToProxyRow(const QAbstractItemModel* pSourceModel, int iSourceRow) const
    {
        QVector<QAbstractItemModel*> vsSourceModels = m_struModelMappingParam.vsSourceModels;
        int iRow = 0;
        // 遍历m_vsModels计算其行数，算出被添加的model
        for (QVector<QAbstractItemModel*>::const_iterator it = vsSourceModels.constBegin(); it != vsSourceModels.constEnd(); ++it)
        {
            QAbstractItemModel* pModel = *it;
            if (NULL == pModel)
            {
                continue;
            }

            if (pModel == pSourceModel)
            {
                return iRow + iSourceRow;
            }

            iRow += pModel->rowCount();
        }

        return iRow;
    }

    // 通过model位置获取行号
    void CCombination_ModelPrivate::GetSourceModelToProxyRow(int iSourceModelIndex, int& iProxyStart, int& iProxyEnd) const
    {
        iProxyStart = 0;
        iProxyEnd = 0;
        QVector<QAbstractItemModel*> vsSourceModels = m_struModelMappingParam.vsSourceModels;
        iSourceModelIndex = qMin(iSourceModelIndex, vsSourceModels.size());
        for (int i = 0; i <= iSourceModelIndex; i++)
        {
            QAbstractItemModel* pSourceModel = vsSourceModels[i];
            if (NULL == pSourceModel)
            {
                continue;
            }

            iProxyStart = iProxyEnd;
            iProxyEnd += pSourceModel->rowCount();
        }
    }

    // 更新下所有的root节点(当添加或删除时)
    void CCombination_ModelPrivate::UpdateAllRootMapping(int iModelIndex)
    {
        Q_Q(const CCombination_Model);
        QVector<QAbstractItemModel*> vsSourceModels = m_struModelMappingParam.vsSourceModels;
        int iRow = 0;
        for (int i = 0, iModelCount = vsSourceModels.size(); i < iModelCount; i++)
        {
            int iProxyRowBegin = iRow;
            QAbstractItemModel* pSourceModel = vsSourceModels[i];
            if (pSourceModel == m_struModelMappingParam.pRemModel)
            {
                continue;
            }
            iRow += pSourceModel->rowCount();

            if (i < iModelIndex)
            {
                continue;
            }

            QHash<MappingParam*, QModelIndex>::const_iterator itMap;
            if (!SourceIndexToIterator(QModelIndex(), pSourceModel, itMap))
            {
                continue;
            }

            MappingParam* pMappingParam = itMap.key();

            // 记录源model行
            int iSourceRowCount = pSourceModel->rowCount();
            pMappingParam->vsSourceRows.clear();
            for (int i = 0; i < iSourceRowCount; ++i) 
            {
                pMappingParam->vsSourceRows.append(i);
            }

            // 记录源model列
            int iSourceColCount = pSourceModel->columnCount();
            pMappingParam->vsSourceColumns.clear();
            for (int i = 0; i < iSourceColCount; ++i) 
            {
                pMappingParam->vsSourceColumns.append(i);
            }
            pMappingParam->iProxyRowBegin = iProxyRowBegin;
            // 记录代理model数据
            pMappingParam->vsProxyRows.resize(iSourceRowCount);
            BuildSourceToProxyMapping(pMappingParam->vsSourceRows, pMappingParam->vsProxyRows);
            pMappingParam->vsProxyColumns.resize(iSourceColCount);
            BuildSourceToProxyMapping(pMappingParam->vsSourceColumns, pMappingParam->vsProxyColumns);
        }
    }

    // 更新pSourceModel的节点关联
    void CCombination_ModelPrivate::UpdateModelMapping(QAbstractItemModel* pSourceModel, const QModelIndex& miSourceParent)
    {
        Q_Q(const CCombination_Model);
        int iRowCount = pSourceModel->rowCount(miSourceParent);
        if (iRowCount > 0)
        {
            // 先创建父关联
            QHash<MappingParam*, QModelIndex>::const_iterator itMap;
            if (!SourceIndexToIterator(miSourceParent, pSourceModel, itMap))
            {
                CreateMapping(miSourceParent, pSourceModel, itMap);
            }
            // 创建子节点的关联
            for (int iRow = 0; iRow < iRowCount; iRow++)
            {
                UpdateModelMapping(pSourceModel, pSourceModel->index(iRow, 0, miSourceParent));
            }
        }
    }

    // 
    void CCombination_ModelPrivate::UpdateRootMappingProxyRowBegin()
    {
        Q_Q(const CCombination_Model);
        QVector<QAbstractItemModel*> vsSourceModels = m_struModelMappingParam.vsSourceModels;
        int iRow = 0;
        for (QVector<QAbstractItemModel*>::iterator it = vsSourceModels.begin(); it != vsSourceModels.end(); ++it)
        {
            QAbstractItemModel* pSourceModel = *it;
            QHash<MappingParam*, QModelIndex>::const_iterator itMap;
            if (SourceIndexToIterator(QModelIndex(), pSourceModel, itMap))
            {
                MappingParam* pMappingParam = itMap.key();
                pMappingParam->iProxyRowBegin = iRow;
                iRow += pMappingParam->vsSourceRows.size();
            }
        }
    }

    // 将代理索引转成源索引
    QModelIndex CCombination_ModelPrivate::ProxyToSource(const QModelIndex& miProxy, bool bFilterRoot) const
    {
        if (!miProxy.isValid())
        {
            return QModelIndex();
        }

        if (miProxy.model() != q_func()) 
        {
            qWarning() << "CCombinationModel: index from wrong model passed to ProxyToSource";
            // 此断言不能存在，过滤model再次设置model时会触发获取索引接口，会报断言
            // Q_ASSERT(!"CCombinationModel: index from wrong model passed to ProxyToSource");
            return QModelIndex();
        }

        // 跟节点
        if (!bFilterRoot && IsValidRootIndex(miProxy))
        {
            return GetRootModel()->index(miProxy.row(), miProxy.column());
        }

        IndexMap::const_iterator it;
        if(!ProxyIndexToIterator(miProxy, it))
        {
            return QModelIndex();
        }

        MappingParam* pMappingParam = it.key();
        int iSourceRow = pMappingParam->ProxyRowToSourceRow(miProxy.row());
        if (iSourceRow == -1)
        {
            return QModelIndex();
        }

        int iSourceCol = pMappingParam->ProxyColumnToSourceColumn(miProxy.column());
        if (iSourceCol == -1)
        {
            return QModelIndex();
        }

        QAbstractItemModel* pSourceModel = pMappingParam->pSourceModel;
        if (NULL == pSourceModel || pSourceModel == m_struModelMappingParam.pRemModel)
        {
            return QModelIndex();
        }

        return pSourceModel->index(iSourceRow, iSourceCol, it.value());
    }

    QModelIndex CCombination_ModelPrivate::SourceToProxy(const QModelIndex& miSource) const
    {
        Q_Q(const CCombination_Model);
        if (!miSource.isValid() && NULL != GetRootModel() && GetRootModel()->rowCount() > 0)
        {
            return q_func()->index(0, 0);
        }

        // 如果是正在删除，返回无效
        if (m_struModelMappingParam.pRemModel != NULL && miSource.model() == m_struModelMappingParam.pRemModel)
        {
            return QModelIndex();
        }

        QModelIndex miSourceParent = miSource.parent();
        IndexMap::const_iterator it;
        if (!SourceIndexToIterator(miSourceParent, miSource.model(), it))
        {
            return QModelIndex();
        }

        MappingParam* pMappingParam = it.key();

        int iProxyRow = SourceRowToProxyRow(pMappingParam->vsSourceRows, miSource.row(), pMappingParam->vsProxyRows, pMappingParam->iProxyRowBegin);
        if (iProxyRow == -1)
        {
            return QModelIndex();
        }
        int iProxyCol = SourceColumnToProxyColumn(pMappingParam->vsSourceColumns, miSource.column(), pMappingParam->vsProxyColumns);
        if (iProxyCol == -1)
        {
            return QModelIndex();
        }

        return CreateIndex(iProxyRow, iProxyCol, it);
    }

    void CCombination_ModelPrivate::SourceModelAboutToBeInserted(const QAbstractItemModel* pSourceModel, const QModelIndex& miSourceParent)
    {
        IndexMap::const_iterator it;
        if (!SourceIndexToIterator(miSourceParent, pSourceModel, it))
        {
            CreateMapping(miSourceParent, pSourceModel, it); // 先创建空的
        }
    }

    void CCombination_ModelPrivate::SourceModelInserted(const QAbstractItemModel* pSourceModel, 
        const QModelIndex& miSourceParent, int iStart, int iEnd, Qt::Orientation enumOrient)
    {
        Q_Q(CCombination_Model);
        if ((iStart < 0) || (iEnd < 0))
        {
            return;
        }
        IndexMap::const_iterator it;
        if (!SourceIndexToIterator(miSourceParent, pSourceModel, it))
        {
            Q_ASSERT(NULL);
            if (!CreateMapping(miSourceParent, pSourceModel, it))
            {
                return;
            }
        }

        MappingParam* pMappingParam = it.key();
        QVector<int>& vsProxys = (enumOrient == Qt::Vertical) ? pMappingParam->vsProxyRows : pMappingParam->vsProxyColumns; // vsProxys []
        QVector<int>& vsSources = (enumOrient == Qt::Vertical) ? pMappingParam->vsSourceRows : pMappingParam->vsSourceColumns;// vsSources []

        int iDeltaItemCount = iEnd - iStart + 1;
        int iOldItemCount = vsProxys.size();

        UpdateChildrenMapping(pSourceModel, miSourceParent, pMappingParam, enumOrient, iStart, iEnd, iDeltaItemCount, false);

        if (iStart < 0 || iStart > vsProxys.size())
        {
            qWarning("CCombinationModel: invalid inserted rows reported by source model");
            RemoveFromMapping(miSourceParent, pSourceModel);
            return;
        }
        vsProxys.insert(iStart, iDeltaItemCount, -1);

        if (iStart < iOldItemCount) 
        {
            int iSourceCount = vsSources.size();
            for (int i = 0; i < iSourceCount; ++i) 
            {
                int iSourceRow = vsSources.at(i);
                if (iSourceRow >= iStart)
                {
                    vsSources.replace(i, iSourceRow + iDeltaItemCount);
                }
            }
            BuildSourceToProxyMapping(vsSources, vsProxys);
        }

        QVector<int> vsSourceItems;
        for (int i = iStart; i <= iEnd; ++i) 
        {
            vsSourceItems.append(i);
        }

        if (pSourceModel->rowCount(miSourceParent) == iDeltaItemCount) 
        {
            QVector<int>& vsOrthogonalSources = (enumOrient == Qt::Horizontal) ? pMappingParam->vsSourceRows : pMappingParam->vsSourceColumns;
            QVector<int>& vsOrthogonalProxys = (enumOrient == Qt::Horizontal) ? pMappingParam->vsProxyRows : pMappingParam->vsProxyColumns;

            if (vsOrthogonalProxys.isEmpty()) 
            {
                const int iOrthoEndCount = (enumOrient == Qt::Horizontal) ? pSourceModel->rowCount(miSourceParent) : pSourceModel->columnCount(miSourceParent);

                vsOrthogonalProxys.resize(iOrthoEndCount);

                for (int i = 0; i < iOrthoEndCount; ++i) 
                {
                    if ((enumOrient == Qt::Horizontal)) 
                    {
                        vsOrthogonalSources.append(i);
                    }
                }
                BuildSourceToProxyMapping(vsOrthogonalSources, vsOrthogonalProxys);
            }
        }

        QModelIndex miProxyParent = q->MapFromSource(miSourceParent);
        if (!miProxyParent.isValid() && miSourceParent.isValid())
        {
            return;
        }

        // 发送即将插入消息
        if (enumOrient == Qt::Vertical)// vsSources[] vsProxys[-1]
        {
            q->beginInsertRows(miProxyParent, iStart + pMappingParam->iProxyRowBegin, iEnd + pMappingParam->iProxyRowBegin);
        }
        else
        {
            q->beginInsertColumns(miProxyParent, iStart, iEnd);
        }

        // 改变源model保存值
        for (int i = iStart; i <= iEnd; ++i)
        {
            vsSources.insert(i, i);
        }

        // 设置关联
        BuildSourceToProxyMapping(vsSources, vsProxys);

        if (!miSourceParent.isValid())
        {
            UpdateRootMappingProxyRowBegin();
        }

        // 数据同步后，发送完成消息
        if (enumOrient == Qt::Vertical)// vsSources[0] vsProxys[0]
        {
            q->endInsertRows();
        }
        else
        {
            q->endInsertColumns();
        }
    }

    void CCombination_ModelPrivate::SourceModelAboutToBeRemoved(QAbstractItemModel* pSourceModel, const QModelIndex& miSourceParent, 
        int iStartSource, int iEndSource, Qt::Orientation enumOrient, bool bRemModel)
    {
        Q_Q(CCombination_Model);
        if ((iStartSource < 0) || (iEndSource < 0))
        {
            return;
        }

        IndexMap::const_iterator it;
        if (!SourceIndexToIterator(miSourceParent, pSourceModel, it))
        {
            if (!CreateMapping(miSourceParent, pSourceModel, it))
            {
                return;
            }
        }

        MappingParam* pMappingParam = it.key();
        QVector<int>& vsProxys = (enumOrient == Qt::Vertical) ? pMappingParam->vsProxyRows : pMappingParam->vsProxyColumns;
        QVector<int>& vsSources = (enumOrient == Qt::Vertical) ? pMappingParam->vsSourceRows : pMappingParam->vsSourceColumns;

        QModelIndex miProxyParent = q->MapFromSource(miSourceParent);
        if (!miProxyParent.isValid() && miSourceParent.isValid())
        {
            return;
        }

        int iProxyStart = SourceRowToProxyRow(vsSources, iStartSource, vsProxys, (enumOrient == Qt::Vertical) ? pMappingParam->iProxyRowBegin : 0);
        if (iProxyStart < 0)
        {
            return;
        }
        int iProxyEnd = SourceRowToProxyRow(vsSources, iEndSource, vsProxys, (enumOrient == Qt::Vertical) ? pMappingParam->iProxyRowBegin : 0);
        if (iProxyEnd < 0)
        {
            return;
        }
        if (enumOrient == Qt::Vertical)
        {
            q->beginRemoveRows(miProxyParent, iProxyStart, iProxyEnd); // vsSources[0,1]  vsProxys[0,1]
        }
        else
        {
            q->beginRemoveColumns(miProxyParent, iProxyStart, iProxyEnd);
        }

        vsSources.remove(iStartSource, iEndSource - iStartSource + 1); // vsSources [1]
        BuildSourceToProxyMapping(vsSources, vsProxys); // vsProxys[-1,0]
        if (!miSourceParent.isValid())
        {
            UpdateRootMappingProxyRowBegin();
        }

        if (enumOrient == Qt::Vertical && bRemModel)
        {
            int iModelIndex = m_struModelMappingParam.vsSourceModels.indexOf(pSourceModel);
            if (iModelIndex != -1)
            {
                m_struModelMappingParam.vsSources.remove(iModelIndex);
            }
            BuildSourceToProxyMapping(m_struModelMappingParam.vsSources, m_struModelMappingParam.vsProxys); // vsProxys[-1,0]
        }
        if (enumOrient == Qt::Vertical)
        {
            q->endRemoveRows(); // vsSources[1] vsProxys[-1,0]
        }
        else
        {
            q->endRemoveColumns();
        }
    }

    void CCombination_ModelPrivate::SourceModelRemoved(QAbstractItemModel* pSourceModel, const QModelIndex& miSourceParent, int iSourceStart, int iSourceEnd,  Qt::Orientation enumOrient)
    {
        Q_Q(CCombination_Model);
        if ((iSourceStart < 0) || (iSourceEnd < 0))
        {
            return;
        }

        IndexMap::const_iterator it;
        if (!SourceIndexToIterator(miSourceParent, pSourceModel, it))
        {
            return;
        }

        MappingParam* pMappingParam = it.key();
        QVector<int>& vsProxys = (enumOrient == Qt::Vertical) ? pMappingParam->vsProxyRows : pMappingParam->vsProxyColumns;
        QVector<int>& vsSources = (enumOrient == Qt::Vertical) ? pMappingParam->vsSourceRows : pMappingParam->vsSourceColumns;

        if (iSourceEnd >= vsProxys.size())
        {
            iSourceEnd = vsProxys.size() - 1;
        }

        int iDeltaItemCount = iSourceEnd - iSourceStart + 1;
        // 删除代理中的数据
        vsProxys.remove(iSourceStart, iDeltaItemCount);

        int iSouceCount = vsSources.size();
        if (iSouceCount != vsProxys.size())
        {
            Q_ASSERT(NULL);
        }

        // 重新计算源索引
        for (int iSource = 0; iSource < iSouceCount; ++iSource)
        {
            int iSourceRow = vsSources.at(iSource);
            if (iSourceRow >= iSourceStart) 
            {
                Q_ASSERT(iSourceRow - iDeltaItemCount >= 0);
                vsSources.replace(iSource, iSourceRow - iDeltaItemCount);
            }
        }
        // 重新关联
        BuildSourceToProxyMapping(vsSources, vsProxys);
        // 更新子child中的关联
        UpdateChildrenMapping(pSourceModel, miSourceParent, pMappingParam, enumOrient, iSourceStart, iSourceEnd, iDeltaItemCount, true);
        if (iSouceCount == 0)
        {
            if (miSourceParent.isValid())
            {
                RemoveFromMapping(miSourceParent, pSourceModel);
            }
            else
            {
                for (int i = 0; i < pMappingParam->vsMappedChildrens.size(); ++i)
                {
                    RemoveFromMapping(pMappingParam->vsMappedChildrens.at(i), pSourceModel);
                }

                pMappingParam->vsMappedChildrens.clear();
            }
        }

        UpdateAllRootMapping(0);
    }

    void CCombination_ModelPrivate::UpdateChildrenMapping(const QAbstractItemModel* pSourceModel, const QModelIndex& miSourceParent, MappingParam* pParentMapping,
        Qt::Orientation enumOrient, int iStart, int iEnd, int iDeltaItemCount, bool bRemove)
    {
        QVector<QPair<QModelIndex, MappingParam*> > vsMovedSourceIndexMappings;
        QVector<QModelIndex>::iterator it2 = pParentMapping->vsMappedChildrens.begin();
        for ( ; it2 != pParentMapping->vsMappedChildrens.end();) 
        {
            const QModelIndex miSourceChild = *it2;
            if (miSourceChild.model() != pSourceModel)
            {
                ++it2;
                continue;
            }

            const int iPos = (enumOrient == Qt::Vertical) ? miSourceChild.row() : miSourceChild.column();
            if (iPos < iStart) 
            {
                ++it2;
            } 
            else if (bRemove && iPos <= iEnd) 
            {
                it2 = pParentMapping->vsMappedChildrens.erase(it2);
                RemoveFromMapping(miSourceChild, pSourceModel);
            } 
            else
            {
                QModelIndex miNew;
                const int iNewPos = bRemove ? iPos - iDeltaItemCount : iPos + iDeltaItemCount;
                if (enumOrient == Qt::Vertical)
                {
                    miNew = pSourceModel->index(iNewPos, miSourceChild.column(), miSourceParent);
                } 
                else 
                {
                    miNew = pSourceModel->index(miSourceChild.row(), iNewPos, miSourceParent);
                }
                *it2 = miNew;
                ++it2;

                MappingParam* pMappingParamC = m_hashSourceIndexMappings.key(miSourceChild);
                if (pMappingParamC == NULL)
                {
                    //Q_ASSERT(pMappingParamC);
                    continue;
                }
                vsMovedSourceIndexMappings.append(QPair<QModelIndex, MappingParam*>(miNew, pMappingParamC));
            }
        }

        QVector<QPair<QModelIndex, MappingParam*> >::iterator it = vsMovedSourceIndexMappings.begin();
        for (; it != vsMovedSourceIndexMappings.end(); ++it) 
        {
            MappingParam* pMappingParamCopy = (*it).second;
            QModelIndex miSourceParentCopy = (*it).first;
            TakeSourceIndexToIterator(miSourceParentCopy, pMappingParamCopy->pSourceModel);
            pMappingParamCopy->iterMap = m_hashSourceIndexMappings.insert(pMappingParamCopy, miSourceParentCopy);
        }
    }

    void CCombination_ModelPrivate::BuildSourceToProxyMapping(const QVector<int>& vsSources, QVector<int>& vsProxys) const
    {
        vsProxys.fill(-1);
        for (int i = 0, iCount = vsSources.size(); i < iCount; ++i)
        {
            vsProxys[vsSources.at(i)] = i;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    void CCombination_ModelPrivate::OnRootDataChanged(const QModelIndex& miSourceTopLeft, const QModelIndex& miSourceBottomRight)
    {
        Q_Q(CCombination_Model);
        if (!miSourceTopLeft.isValid() || !miSourceBottomRight.isValid())
        {
            return;
        }

        QModelIndex miProxyTopLeft = q->index(miSourceTopLeft.row(), miSourceTopLeft.column());
        QModelIndex miProxyBottomRight = q->index(miSourceBottomRight.row(), miSourceBottomRight.column());
        emit q->dataChanged(miProxyTopLeft, miProxyBottomRight);
    }

    void CCombination_ModelPrivate::OnRootHeaderDataChanged(Qt::Orientation enumOrientation, int iStart, int iEnd)
    {
        Q_Q(CCombination_Model);
        emit q->headerDataChanged(enumOrientation, iStart, iEnd);
    }

    void CCombination_ModelPrivate::OnRootRowsAboutToBeInserted(const QModelIndex& miSourceParent, int iStart, int iEnd)
    {
        CreateRootMapping();
    }

    void CCombination_ModelPrivate::OnRootRowsInserted(const QModelIndex& miSourceParent, int iStart, int iEnd)
    {
        Q_Q(CCombination_Model);
        MappingParam* pMappingParam = CreateRootMapping();
        if (NULL == pMappingParam)
        {
            Q_ASSERT(NULL);
            return;
        }

        Qt::Orientation enumOrient = Qt::Vertical;
        QAbstractItemModel* pSourceModel = pMappingParam->pSourceModel;
        QVector<int>& vsProxys = (enumOrient == Qt::Vertical) ? pMappingParam->vsProxyRows : pMappingParam->vsProxyColumns; // vsProxys []
        QVector<int>& vsSources = (enumOrient == Qt::Vertical) ? pMappingParam->vsSourceRows : pMappingParam->vsSourceColumns;// vsSources []

        int iDeltaItemCount = iEnd - iStart + 1;
        int iOldItemCount = vsProxys.size();

        //UpdateChildrenMapping(pSourceModel, miSourceParent, pMappingParam, enumOrient, iStart, iEnd, delta_item_count, false);
        vsProxys.insert(iStart, iDeltaItemCount, -1);

        if (iStart < iOldItemCount) 
        {
            int iCount = vsSources.size();
            for (int i = 0; i < iCount; ++i) 
            {
                int iSourceRow = vsSources.at(i);
                if (iSourceRow >= iStart)
                {
                    vsSources.replace(i, iSourceRow + iDeltaItemCount);
                }
            }
            BuildSourceToProxyMapping(vsSources, vsProxys);
        }

        QVector<int> vsSourceItems;
        for (int i = iStart; i <= iEnd; ++i) 
        {
            vsSourceItems.append(i);
        }

        if (pSourceModel->rowCount(miSourceParent) == iDeltaItemCount) 
        {
            QVector<int>& vsOrthogonalSources = (enumOrient == Qt::Horizontal) ? pMappingParam->vsSourceRows : pMappingParam->vsSourceColumns;
            QVector<int>& vsOrthogonalProxys = (enumOrient == Qt::Horizontal) ? pMappingParam->vsProxyRows : pMappingParam->vsProxyColumns;

            if (vsOrthogonalProxys.isEmpty()) 
            {
                const int iOrthoEndCount = (enumOrient == Qt::Horizontal) ? pSourceModel->rowCount(miSourceParent) : pSourceModel->columnCount(miSourceParent);

                vsOrthogonalProxys.resize(iOrthoEndCount);

                for (int iOrtho = 0; iOrtho < iOrthoEndCount; ++iOrtho) 
                {
                    if ((enumOrient == Qt::Horizontal)) 
                    {
                        vsOrthogonalSources.append(iOrtho);
                    }
                }
                BuildSourceToProxyMapping(vsOrthogonalSources, vsOrthogonalProxys);
            }
        }

        QModelIndex miProxyParent = q->MapFromSource(miSourceParent);
        if (!miProxyParent.isValid() && miSourceParent.isValid())
        {
            return;
        }

        // 发送即将插入消息
        if (enumOrient == Qt::Vertical)// vsSources[] vsProxys[-1]
        {
            q->beginInsertRows(miProxyParent, iStart + pMappingParam->iProxyRowBegin, iEnd + pMappingParam->iProxyRowBegin);
        }
        else
        {
            q->beginInsertColumns(miProxyParent, iStart, iEnd);
        }

        // 改变源model保存值
        for (int i = iStart; i <= iEnd; ++i)
        {
            vsSources.insert(i, i);
        }

        // 设置关联
        BuildSourceToProxyMapping(vsSources, vsProxys);

        // 数据同步后，发送完成消息
        if (enumOrient == Qt::Vertical)// vsSources[0] vsProxys[0]
        {
            q->endInsertRows();
        }
        else
        {
            q->endInsertColumns();
        }

        UpdateAllRootMapping(0);
    }

    void CCombination_ModelPrivate::OnRootRowsAboutToBeRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd)
    {
        Q_Q(CCombination_Model);
        Q_ASSERT(!miSourceParent.isValid());
        MappingParam* pMappingParam = CreateRootMapping();
        if (NULL == pMappingParam)
        {
            return;
        }

        emit q->beginRemoveRows(QModelIndex(), iStart, iEnd);

        pMappingParam->vsSourceRows.remove(iStart, iEnd - iStart + 1); // vsSources [1]
        BuildSourceToProxyMapping(pMappingParam->vsSourceRows, pMappingParam->vsProxyRows); // vsProxys[-1,0]
        OnClearMapping();

        emit q->endRemoveRows();
    }

    void CCombination_ModelPrivate::OnRootRowsRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd)
    {
        MappingParam* pMappingParam = CreateRootMapping();
        if (NULL != pMappingParam)
        {
            pMappingParam->vsProxyRows.clear();
            pMappingParam->vsSourceRows.clear();
        }
    }

    void CCombination_ModelPrivate::OnRootModelDestroyed(QObject* pObject)
    {
        if (NULL == pObject)
        {
            return;
        }

        m_bRemRootModel = true;
        qWarning("---CCombination_ModelPrivate::OnRootModelDestroyed---");
        MappingParam* pMappingParam = CreateRootMapping();
        if (NULL != pMappingParam)
        {
            int iRowCount = pMappingParam->vsSourceRows.size();
            OnRootRowsAboutToBeRemoved(QModelIndex(), 0, iRowCount - 1);
            OnRootRowsRemoved(QModelIndex(), 0, iRowCount - 1);
        }

        pObject->disconnect(this);
        m_pRootModel = NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    void CCombination_ModelPrivate::OnSourceDataChanged(const QModelIndex& miSourceTopLeft, const QModelIndex& miSourceBottomRight)
    {
        Q_Q(CCombination_Model);
        if (!miSourceTopLeft.isValid() || !miSourceBottomRight.isValid())
        {
            return;
        }

        // 如果是正在删除，返回无效
        if (m_struModelMappingParam.pRemModel != NULL && miSourceTopLeft.model() == m_struModelMappingParam.pRemModel)
        {
            return;
        }

        QModelIndex miSourceParent = miSourceTopLeft.parent();
        QAbstractItemModel* pSourceModel = qobject_cast<QAbstractItemModel*>(q->sender());
        IndexMap::const_iterator it;
        if (!SourceIndexToIterator(miSourceParent, pSourceModel, it))
        {
            if (!CreateMapping(miSourceParent, pSourceModel, it))
            {
                return;
            }
        }

        MappingParam* pMappingParam = it.key();

        QVector<int> vsSourceRowsChanges;
        int iEnd = qMin(miSourceBottomRight.row(), pMappingParam->vsProxyRows.count() - 1);
        for (int source_row = miSourceTopLeft.row(); source_row <= iEnd; ++source_row) 
        {
            if (SourceRowToProxyRow(pMappingParam->vsSourceRows, source_row, pMappingParam->vsProxyRows, pMappingParam->iProxyRowBegin) != -1)
            {
                vsSourceRowsChanges.append(source_row);
            }
        }

        if (!vsSourceRowsChanges.isEmpty())
        {
            int iProxyStartRow = SourceRowToProxyRow(pMappingParam->vsSourceRows, miSourceTopLeft.row(), pMappingParam->vsProxyRows, pMappingParam->iProxyRowBegin);
            int iProxyEndRow = SourceRowToProxyRow(pMappingParam->vsSourceRows, miSourceBottomRight.row(), pMappingParam->vsProxyRows, pMappingParam->iProxyRowBegin);
            if (iProxyEndRow >= 0) 
            {
                int iSourceLeftColumn = miSourceTopLeft.column();
                while (iSourceLeftColumn < miSourceBottomRight.column()
                    && SourceColumnToProxyColumn(pMappingParam->vsSourceColumns, iSourceLeftColumn, pMappingParam->vsProxyColumns) == -1)
                {
                    ++iSourceLeftColumn;
                }
                const QModelIndex miProxyTopLeft = CreateIndex(
                    iProxyStartRow, SourceColumnToProxyColumn(pMappingParam->vsSourceColumns, iSourceLeftColumn, pMappingParam->vsProxyColumns), it);
                int iSourceRightColumn = miSourceBottomRight.column();
                while (iSourceRightColumn > miSourceTopLeft.column()
                    && SourceColumnToProxyColumn(pMappingParam->vsSourceColumns, iSourceRightColumn, pMappingParam->vsProxyColumns) == -1)
                {
                    --iSourceRightColumn;
                }

                const QModelIndex miProxyBottomRight = CreateIndex(iProxyEndRow, q->columnCount() - 1 // 因为有可能存在映射，需做数据更新范围扩大到整列
                    /*SourceColumnToProxyColumn(pMappingParam->vsSourceColumns, iSourceRightColumn, pMappingParam->vsProxyColumns)*/, it);
                emit q->dataChanged(miProxyTopLeft, miProxyBottomRight);
            }
        }
    }

    void CCombination_ModelPrivate::OnSourceAboutToBeReset()
    {
        Q_ASSERT(NULL);
    }

    void CCombination_ModelPrivate::OnSourceReset()
    {
        Q_ASSERT(NULL);
    }

    void CCombination_ModelPrivate::OnSourceLayoutAboutToBeChanged()
    {
        Q_ASSERT(NULL);
    }

    void CCombination_ModelPrivate::OnSourceLayoutChanged()
    {
        Q_ASSERT(NULL);
    }

    void CCombination_ModelPrivate::OnSourceRowsAboutToBeInserted(const QModelIndex& miSourceParent, int iStart, int iEnd)
    {
        Q_UNUSED(iStart);
        Q_UNUSED(iEnd);
        Q_Q(CCombination_Model);
        SourceModelAboutToBeInserted(qobject_cast<const QAbstractItemModel*>(q->sender()), miSourceParent);
    }

    void CCombination_ModelPrivate::OnSourceRowsInserted(const QModelIndex& miSourceParent, int iStart, int iEnd)
    {
        Q_Q(CCombination_Model);
        SourceModelInserted(qobject_cast<const QAbstractItemModel*>(q->sender()), miSourceParent, iStart, iEnd, Qt::Vertical);
    }

    void CCombination_ModelPrivate::OnSourceRowsAboutToBeRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd)
    {
        Q_Q(CCombination_Model);
        QAbstractItemModel* pSouceModel = qobject_cast<QAbstractItemModel*>(q->sender());
        SourceModelAboutToBeRemoved(pSouceModel, miSourceParent, iStart, iEnd, Qt::Vertical);
    }

    void CCombination_ModelPrivate::OnSourceRowsRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd)
    {
        Q_Q(CCombination_Model);
        QAbstractItemModel* pSouceModel = qobject_cast<QAbstractItemModel*>(q->sender());
        SourceModelRemoved(pSouceModel, miSourceParent, iStart, iEnd, Qt::Vertical);
    }

    void CCombination_ModelPrivate::OnSourceColumnsAboutToBeInserted(const QModelIndex& miSourceParent, int iStart, int iEnd)
    {
        Q_UNUSED(iStart);
        Q_UNUSED(iEnd);
        Q_Q(const CCombination_Model);
        const QAbstractItemModel* pSourceModel = qobject_cast<const QAbstractItemModel*>(q->sender());
        IndexMap::const_iterator it;
        if (!SourceIndexToIterator(miSourceParent, pSourceModel, it))
        {
            if (!CreateMapping(miSourceParent, pSourceModel, it))
            {
                return;
            }
        }
    }

    void CCombination_ModelPrivate::OnSourceColumnsInserted(const QModelIndex& miSourceParent, int iStart, int iEnd)
    {
        Q_Q(const CCombination_Model);
        SourceModelInserted(qobject_cast<const QAbstractItemModel*>(q->sender()), miSourceParent, iStart, iEnd, Qt::Horizontal);
    }

    void CCombination_ModelPrivate::OnSourceColumnsAboutToBeRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd)
    {
        Q_Q(CCombination_Model);
        QAbstractItemModel* pSouceModel = qobject_cast<QAbstractItemModel*>(q->sender());
        SourceModelAboutToBeRemoved(pSouceModel, miSourceParent, iStart, iEnd, Qt::Horizontal);
    }

    void CCombination_ModelPrivate::OnSourceColumnsRemoved(const QModelIndex& miSourceParent, int iStart, int iEnd)
    {
        Q_Q(CCombination_Model);
        QAbstractItemModel* pSouceModel = qobject_cast<QAbstractItemModel*>(q->sender());
        SourceModelRemoved(pSouceModel, miSourceParent, iStart, iEnd, Qt::Horizontal);
    }

    CCombination_Model::CCombination_Model(QObject* pParent)
        : QAbstractItemModel(pParent),
        d_ptr(new CCombination_ModelPrivate)
    {
        d_ptr->q_ptr = this;
    }

    CCombination_Model::~CCombination_Model()
    {
        Q_D(CCombination_Model);
        d->Clear();
    }

    // 设置纵列数量(纵列数不依赖源model的纵列改变，需控制添加)
    void CCombination_Model::SetColumnCount(int iCounts)
    {
        int iColumnCount = columnCount();
        if (iColumnCount == iCounts)
        {
            return;
        }
        if (iColumnCount < iCounts)
        {
            insertColumns(qMax(iColumnCount, 0), iCounts - iColumnCount);
        }
        else
        {
            removeColumns(qMax(iCounts, 0), iColumnCount - iCounts);
        }
    }

    // 设置root节点,所有model都加到此节点下
    void CCombination_Model::SetRootModel(QAbstractItemModel* pRootModel)
    {
        Q_D(CCombination_Model);
        Q_ASSERT(pRootModel);
        if (d->m_struModelMappingParam.vsSourceModels.size() > 0)
        {
            Q_ASSERT(NULL);
            return;
        }

        // 如果默认存在，说明已经启用默认
        if (NULL != d->m_pDefaultRootModel)
        {
            Q_ASSERT(NULL);
            return;
        }

        if (NULL != d->m_pRootModel)
        {
            Q_ASSERT(NULL);
            return;
        }

        d->m_pRootModel = pRootModel;

        ConnectRoot(pRootModel);
    }

    // 获取root节点model
    QAbstractItemModel* CCombination_Model::GetRootModel() const
    {
        Q_D(const CCombination_Model);
        return d->GetRootModel();
    }

    // 连接root的信号槽
    void CCombination_Model::ConnectRoot(QAbstractItemModel* pRootModel) const
    {
        connect(pRootModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this, SLOT(OnRootDataChanged(QModelIndex, QModelIndex)));

        connect(pRootModel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            this, SLOT(OnRootHeaderDataChanged(Qt::Orientation,int,int)));

        connect(pRootModel, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)),
            this, SLOT(OnRootRowsAboutToBeInserted(QModelIndex, int, int)));

        connect(pRootModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
            this, SLOT(OnRootRowsInserted(QModelIndex, int, int)));

        /* connect(pRootModel, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
        this, SLOT(OnRootColumnsAboutToBeInserted(QModelIndex,int,int)));

        connect(pRootModel, SIGNAL(columnsInserted(QModelIndex,int,int)),
        this, SLOT(OnRootColumnsInserted(QModelIndex,int,int)));*/

        connect(pRootModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(OnRootRowsAboutToBeRemoved(QModelIndex,int,int)));

        connect(pRootModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(OnRootRowsRemoved(QModelIndex,int,int)));

        /*connect(pRootModel, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
        this, SLOT(OnRootColumnsAboutToBeRemoved(QModelIndex,int,int)));

        connect(pRootModel, SIGNAL(columnsRemoved(QModelIndex,int,int)),
        this, SLOT(OnRootColumnsRemoved(QModelIndex,int,int)));

        connect(pRootModel, SIGNAL(layoutAboutToBeChanged()),
        this, SLOT(OnRootLayoutAboutToBeChanged()));

        connect(pRootModel, SIGNAL(layoutChanged()),
        this, SLOT(OnRootLayoutChanged()));

        connect(pRootModel, SIGNAL(modelAboutToBeReset()), this, SLOT(OnRootAboutToBeReset()));
        connect(pRootModel, SIGNAL(modelReset()), this, SLOT(OnRootReset()));*/

        connect(pRootModel, SIGNAL(destroyed(QObject*)), this, SLOT(OnRootModelDestroyed(QObject*)));
    }

    bool CCombination_Model::AddModel(QAbstractItemModel* pSourceModel)
    {
        Q_D(CCombination_Model);
        return InsertModel(d->m_struModelMappingParam.vsSourceModels.size(), pSourceModel);
    }

    bool CCombination_Model::InsertModel(int iIndex, QAbstractItemModel* pSourceModel)
    {
        Q_D(CCombination_Model);

        QVector<QAbstractItemModel*>& vsSourceModels = d->m_struModelMappingParam.vsSourceModels;
        if (iIndex < 0 || iIndex > vsSourceModels.size())
        {
            return false;
        }

        if (NULL == pSourceModel)
        {
            Q_ASSERT(NULL);
            return false;
        }

        if (vsSourceModels.contains(pSourceModel))
        {
            Q_ASSERT(NULL);
            return false;
        }

        if (columnCount() == 0)
        {
            Q_ASSERT("CCombinationModel::InsertModel: columnCount() == 0");
            return false;
        }

        int iProxyStart = 0;
        int iProxyEnd = 0;
        d->GetSourceModelToProxyRow(iIndex - 1, iProxyStart, iProxyEnd);
        IndexMap::const_iterator it;
        if (d->CreateMapping(QModelIndex(), pSourceModel, it))
        {
            CCombination_ModelPrivate::MappingParam* pMappingParam = it.key();
            pMappingParam->vsSourceRows.clear();
            pMappingParam->vsProxyRows.clear();
        }

        int iSourceRowCount = pSourceModel->rowCount();
        if (iSourceRowCount > 0)
        {
            beginInsertRows(MapFromSource(QModelIndex()), iProxyEnd, iProxyEnd + iSourceRowCount - 1);
        }

        d->m_struModelMappingParam.vsSourceModels.insert(iIndex, pSourceModel);
        d->m_struModelMappingParam.vsProxys.insert(iIndex, -1);

        connect(pSourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(OnSourceDataChanged(QModelIndex,QModelIndex)));

        connect(pSourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(OnSourceRowsAboutToBeInserted(QModelIndex,int,int)));

        connect(pSourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(OnSourceRowsInserted(QModelIndex,int,int)));

        connect(pSourceModel, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(OnSourceColumnsAboutToBeInserted(QModelIndex,int,int)));

        connect(pSourceModel, SIGNAL(columnsInserted(QModelIndex,int,int)),
            this, SLOT(OnSourceColumnsInserted(QModelIndex,int,int)));

        connect(pSourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(OnSourceRowsAboutToBeRemoved(QModelIndex,int,int)));

        connect(pSourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(OnSourceRowsRemoved(QModelIndex,int,int)));

        connect(pSourceModel, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(OnSourceColumnsAboutToBeRemoved(QModelIndex,int,int)));

        connect(pSourceModel, SIGNAL(columnsRemoved(QModelIndex,int,int)),
            this, SLOT(OnSourceColumnsRemoved(QModelIndex,int,int)));

        connect(pSourceModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(OnSourceLayoutAboutToBeChanged()));

        connect(pSourceModel, SIGNAL(layoutChanged()),
            this, SLOT(OnSourceLayoutChanged()));

        connect(pSourceModel, SIGNAL(modelAboutToBeReset()), this, SLOT(OnSourceAboutToBeReset()));
        connect(pSourceModel, SIGNAL(modelReset()), this, SLOT(OnSourceReset()));

        connect(pSourceModel, SIGNAL(destroyed(QObject*)), this, SLOT(OnSourceModelDestroyed(QObject*)));

        d->UpdateAllRootMapping(iIndex);
        d->m_struModelMappingParam.UpdateMapping();
        // 更新被添加进来子的节点关联
        d->UpdateModelMapping(pSourceModel);
        if (iSourceRowCount > 0)
        {
            endInsertRows();
        }
        return true;
    }

    // 在源model添加顺序队列位置iIndex插入
//     bool CCombination_Model::InsertModels(int iIndex, QList<QAbstractItemModel*> listSourceModels)
//     {
//         Q_D(CCombination_Model);
// 
//         QVector<QAbstractItemModel*>& vsSourceModels = d->m_struModelMappingParam.vsSourceModels;
//         if (iIndex < 0 || iIndex > vsSourceModels.size())
//         {
//             return false;
//         }
// 
//         if (listSourceModels.size() == 0)
//         {
//             return false;
//         }
// 
//         if (columnCount() == 0)
//         {
//             Q_ASSERT("CCombinationModel::InsertModel: columnCount() == 0");
//             return false;
//         }
// 
//         int iProxyStart = 0;
//         int iProxyEnd = 0;
//         d->GetSourceModelToProxyRow(iIndex - 1, iProxyStart, iProxyEnd);
//         int iSourceRowCount = 0;
//         // 先创建空的映射
//         for (QList<QAbstractItemModel*>::iterator itModel = listSourceModels.begin(); itModel != listSourceModels.end(); ++itModel)
//         {
//             CCombination_ModelPrivate::MappingParam* pMappingParam = new CCombination_ModelPrivate::MappingParam;
//             pMappingParam->pSourceModel = *itModel;
// 
//             // 记录源model行
//             int iSourceRows = pMappingParam->pSourceModel->rowCount();
//             int iRow = d->GetSourceRowToProxyRow(*itModel, 0);
// 
//             pMappingParam->iProxyRowBegin = iRow;
//             iSourceRowCount += iSourceRows;
// 
// 
//             IndexMap::const_iterator it = IndexMap::const_iterator(d->m_hashSourceIndexMappings.insert(pMappingParam, QModelIndex()));
//             pMappingParam->iterMap = it;
//         }
// 
//         if (iSourceRowCount > 0)
//         {
//             beginInsertRows(MapFromSource(QModelIndex()), iProxyEnd, iProxyEnd + iSourceRowCount - 1);
//         }
// 
//         int iPos = 0;
//         for (QList<QAbstractItemModel*>::iterator itModel = listSourceModels.begin(); itModel != listSourceModels.end(); ++itModel)
//         {
//             QAbstractItemModel* pSourceModel = *itModel;
//             d->m_struModelMappingParam.vsSourceModels.insert(iIndex + iPos, pSourceModel);
//             d->m_struModelMappingParam.vsProxys.insert(iIndex + iPos, -1);
//             connect(pSourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
//                 this, SLOT(OnSourceDataChanged(QModelIndex,QModelIndex)));
// 
//             connect(pSourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
//                 this, SLOT(OnSourceRowsAboutToBeInserted(QModelIndex,int,int)));
// 
//             connect(pSourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
//                 this, SLOT(OnSourceRowsInserted(QModelIndex,int,int)));
// 
//             connect(pSourceModel, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
//                 this, SLOT(OnSourceColumnsAboutToBeInserted(QModelIndex,int,int)));
// 
//             connect(pSourceModel, SIGNAL(columnsInserted(QModelIndex,int,int)),
//                 this, SLOT(OnSourceColumnsInserted(QModelIndex,int,int)));
// 
//             connect(pSourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
//                 this, SLOT(OnSourceRowsAboutToBeRemoved(QModelIndex,int,int)));
// 
//             connect(pSourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
//                 this, SLOT(OnSourceRowsRemoved(QModelIndex,int,int)));
// 
//             connect(pSourceModel, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
//                 this, SLOT(OnSourceColumnsAboutToBeRemoved(QModelIndex,int,int)));
// 
//             connect(pSourceModel, SIGNAL(columnsRemoved(QModelIndex,int,int)),
//                 this, SLOT(OnSourceColumnsRemoved(QModelIndex,int,int)));
// 
//             connect(pSourceModel, SIGNAL(layoutAboutToBeChanged()),
//                 this, SLOT(OnSourceLayoutAboutToBeChanged()));
// 
//             connect(pSourceModel, SIGNAL(layoutChanged()),
//                 this, SLOT(OnSourceLayoutChanged()));
// 
//             connect(pSourceModel, SIGNAL(modelAboutToBeReset()), this, SLOT(OnSourceAboutToBeReset()));
//             connect(pSourceModel, SIGNAL(modelReset()), this, SLOT(OnSourceReset()));
// 
//             connect(pSourceModel, SIGNAL(destroyed(QObject*)), this, SLOT(OnSourceModelDestroyed(QObject*)));
// 
//             iPos++;
//         }
// 
//         d->UpdateAllRootMapping(iIndex);
//         d->m_struModelMappingParam.UpdateMapping();
//         for (QList<QAbstractItemModel*>::iterator itModel = listSourceModels.begin(); itModel != listSourceModels.end(); ++itModel)
//         {
//             // 更新被添加进来子的节点关联
//             d->UpdateModelMapping(*itModel);
//         }
//         
//         if (iSourceRowCount > 0)
//         {
//             endInsertRows();
//         }
//         return true;
//     }

    QModelIndex CCombination_Model::index(int iRow, int iColumn, const QModelIndex& miProxyParent) const
    {
        Q_D(const CCombination_Model);

        if (iRow < 0 || iColumn < 0)
        {
            return QModelIndex();
        }

        if (d->IsValidRootParentIndex(miProxyParent))
        {
            // 如果m_pRootMappingParam存在且无效（将要删除获取无效）
            if (NULL != d->m_pRootMappingParam && d->m_pRootMappingParam->ProxyRowToSourceRow(iRow) == -1)
            {
                return QModelIndex();
            }

            // 不存在就创建
            d->CreateRootMapping();

            return QAbstractItemModel::createIndex(iRow, iColumn);
        }
        else
        {
            QModelIndex miSourceParent = d->ProxyToSource(miProxyParent, true);
            // 先查看是否存在（防止即将删除通过GetSourceModelByRow获取的数据不对）
            IndexMap::const_iterator it;
            if (d->SourceIndexToIterator(miSourceParent, iRow, it))
            {
                CCombination_ModelPrivate::MappingParam* pMappingParam = it.key();
                if (pMappingParam->ProxyRowToSourceRow(iRow) == -1)
                {
                    return QModelIndex();
                }

                return d->CreateIndex(iRow, iColumn, it);
            }

            const QAbstractItemModel* pSourceModel = miSourceParent.model();
            if (NULL == pSourceModel)
            {
                pSourceModel = d->GetSourceModelByProxyRow(iRow);
            }

            if (d->SourceIndexToIterator(miSourceParent, pSourceModel, it) || d->CreateMapping(miSourceParent, pSourceModel, it))
            {
                CCombination_ModelPrivate::MappingParam* pMappingParam = it.key();
                if (pMappingParam->ProxyRowToSourceRow(iRow) != -1)
                {
                    return d->CreateIndex(iRow, iColumn, it);
                }
            }
        }

        return QModelIndex();
    }

    QModelIndex CCombination_Model::parent(const QModelIndex& miChild) const
    {
        Q_D(const CCombination_Model);

        if (!d->IsIndexValid(miChild))
        {
            return QModelIndex();
        }

        IndexMap::const_iterator it;
        if (!d->ProxyIndexToIterator(miChild, it))
        {
            return QModelIndex();
        }

        QModelIndex miSourceParent = it.value();
        QModelIndex miProxyParent = MapFromSource(miSourceParent);
        return miProxyParent;
    }

    int CCombination_Model::rowCount(const QModelIndex& miParent) const
    {
        Q_D(const CCombination_Model);
        int iRowCount = 0;
        if (d->IsValidRootParentIndex(miParent))
        {
            iRowCount = d->GetRootModel()->rowCount(); 
            iRowCount = qMin(1, iRowCount);// 暂时只支持一级
        }
        else if (d->IsValidRootIndex(miParent))
        {
            // 如果无效,说明是root节点,需遍历所有的源model的节点数
            for (QHash<CCombination_ModelPrivate::MappingParam*, QModelIndex>::const_iterator it = d->m_hashSourceIndexMappings.constBegin(); 
                it != d->m_hashSourceIndexMappings.constEnd(); ++it)
            {
                QModelIndex miSource = it.value();
                if (!miSource.isValid())
                {
                    CCombination_ModelPrivate::MappingParam* pMappingParam = it.key();
                    iRowCount += pMappingParam->vsSourceRows.count();
                }
            }
        }
        else
        {
            QModelIndex miSourceParent = d->ProxyToSource(miParent);
            if (miSourceParent.isValid()) 
            {
                const QAbstractItemModel* pSourceModel = miSourceParent.model();
                IndexMap::const_iterator it;
                if (!d->SourceIndexToIterator(miSourceParent, pSourceModel, it))
                {
                    if (pSourceModel->hasChildren(miSourceParent))
                    {
                        if (!d->CreateMapping(miSourceParent, pSourceModel, it))
                        {
                            return iRowCount;
                        }
                    }
                    else
                    {
                        return iRowCount;
                    }
                }
                CCombination_ModelPrivate::MappingParam* pMappingParam = it.key();
                iRowCount = pMappingParam->vsSourceRows.count();
            }
            else
            {
                // 如果无效,说明是root节点,需遍历所有的源model的节点数
                for (QHash<CCombination_ModelPrivate::MappingParam*, QModelIndex>::const_iterator it = d->m_hashSourceIndexMappings.constBegin(); 
                    it != d->m_hashSourceIndexMappings.constEnd(); ++it)
                {
                    QModelIndex miSource = it.value();
                    if (!miSource.isValid())
                    {
                        CCombination_ModelPrivate::MappingParam* pMappingParam = it.key();
                        iRowCount += pMappingParam->vsSourceRows.count();
                    }
                }
            }
        }

        return iRowCount;
    }

    int CCombination_Model::columnCount(const QModelIndex& miParent) const
    {
        Q_D(const CCombination_Model);

        // 暂时默认1
        if (NULL != d->GetRootModel())
        {
            return d->GetRootModel()->columnCount();
        }
        else
        {
            Q_ASSERT(NULL);
        }

        return 0;
    }

    bool CCombination_Model::hasChildren(const QModelIndex& miProxyParent) const
    {
        Q_D(const CCombination_Model);

        if (d->IsValidRootParentIndex(miProxyParent))
        {   // 存在root节点的父节点
            if (d->GetRootModel()->hasChildren())
            {
                return true;
            }
        }
        else if (d->IsValidRootIndex(miProxyParent))
        {
            // 存在root节点
            // 需遍历所有的源model的节点数
            for (QHash<CCombination_ModelPrivate::MappingParam*, QModelIndex>::const_iterator it = d->m_hashSourceIndexMappings.constBegin(); 
                it != d->m_hashSourceIndexMappings.constEnd(); ++it)
            {
                QModelIndex miSource = it.value();
                if (!miSource.isValid())
                {
                    CCombination_ModelPrivate::MappingParam* pMappingParam = it.key();
                    if (pMappingParam->vsSourceRows.size() > 0)
                    {
                        return true;
                    }
                }
            }
        }
        else
        {
            QModelIndex miSourceParent = d->ProxyToSource(miProxyParent);
            if (miSourceParent.isValid()) 
            {
                // 存在源
                const QAbstractItemModel* pSourceModel = miSourceParent.model();
                if (NULL == pSourceModel || !pSourceModel->hasChildren(miSourceParent))
                {
                    return false;
                }

                IndexMap::const_iterator it;
                if (!d->SourceIndexToIterator(miSourceParent, pSourceModel, it))
                {
                    if (!d->CreateMapping(miSourceParent, pSourceModel, it))
                    {
                        return false;
                    }
                }

                CCombination_ModelPrivate::MappingParam* pMappingParam = it.key();
                return pMappingParam->vsSourceRows.count() != 0 && pMappingParam->vsSourceColumns.count() != 0;
            }
            else
            {
                // 如果无效,说明是root节点,需遍历所有的源model的节点数
                for (QHash<CCombination_ModelPrivate::MappingParam*, QModelIndex>::const_iterator it = d->m_hashSourceIndexMappings.constBegin(); 
                    it != d->m_hashSourceIndexMappings.constEnd(); ++it)
                {
                    QModelIndex miSource = it.value();
                    if (!miSource.isValid())
                    {
                        CCombination_ModelPrivate::MappingParam* pMappingParam = it.key();
                        if (pMappingParam->vsSourceRows.size() > 0)
                        {
                            return true;
                        }
                    }
                }
            }
        }
        
        return false;
    }

    QVariant CCombination_Model::data(const QModelIndex& miProxy, int iRole) const
    {
        int iTargetColumn = miProxy.column();
        int iTargetRole = iRole;
        QModelIndex miMapping = miProxy;
        if (GetMapping(miProxy.column(), iRole, iTargetColumn, iTargetRole))
        {
            miMapping = index(miProxy.row(), iTargetColumn, miProxy.parent());
        }

        if (iRole == Qt::ForegroundRole)
        {
            // 若存在此CFilterStringListRole值，特殊处理文字颜色（过滤定位中使用此功能）
            QVariant vt = headerData(0, Qt::Horizontal, CFilterStringListRole);
            if (vt.isValid())
            {
                QString strDisplay = miProxy.data(Qt::DisplayRole).toString();

                // 匹配字符
                QStringList strList = vt.toStringList();
                for (QStringList::const_iterator it = strList.constBegin(); it != strList.constEnd(); ++it)
                {
                    QString strFilter = *it;
                    if (!strFilter.isNull() && strDisplay.contains(strFilter, Qt::CaseInsensitive))
                    {
                        return QColor(220, 82, 82);
                    }
                }
            }
        }

        Q_D(const CCombination_Model);
        if (d->IsValidRootIndex(miMapping))
        {
            return d->GetRootModel()->data(d->GetRootModel()->index(0, 0), iTargetRole);
        }
        else
        {
            IndexMap::const_iterator it;
            if (d->ProxyIndexToIterator(miMapping, it))
            {
                CCombination_ModelPrivate::MappingParam* pMappingParam = it.key();
                const QAbstractItemModel* pSourceModel = pMappingParam->pSourceModel;
                if (pSourceModel != d->m_struModelMappingParam.pRemModel)
                {
                    QModelIndex miSourceIndex = d->ProxyToSource(miMapping);
                    return pSourceModel->data(miSourceIndex, iTargetRole);
                }
            }
            else
            {

            }
        }

        return QVariant();
    }

    bool CCombination_Model::setData(const QModelIndex& miProxy, const QVariant& vtValue, int iRole)
    {
        Q_D(const CCombination_Model);
        if (d->IsValidRootIndex(miProxy))
        {
            return d->GetRootModel()->setData(d->GetRootModel()->index(0, 0), vtValue, iRole);
        }
        else
        {
            IndexMap::const_iterator it;
            if (d->ProxyIndexToIterator(miProxy, it))
            {
                CCombination_ModelPrivate::MappingParam* pMappingParam = it.key();
                QAbstractItemModel* pSourceModel = pMappingParam->pSourceModel;
                QModelIndex miSourceIndex = d->ProxyToSource(miProxy);
                return pSourceModel->setData(miSourceIndex, vtValue, iRole);
            }
        }

        return false;
    }

    QVariant CCombination_Model::headerData(int iSection,  Qt::Orientation enumOrientation, int iRole) const
    {
        Q_D(const CCombination_Model);

        if (NULL != d->GetRootModel())
        {
            return d->GetRootModel()->headerData(iSection, enumOrientation, iRole);
        }
        else
        {
            Q_ASSERT(NULL);
        }

        return QVariant();
    }

    bool CCombination_Model::setHeaderData(int iSection,  Qt::Orientation enumOrientation, const QVariant& vtValue, int iRole)
    {
        Q_D(const CCombination_Model);

        if (NULL != d->GetRootModel())
        {
			//表头列宽匹配4K屏
			QVariant qvariant_value = vtValue;
			if (enumOrientation == Qt::Horizontal && iRole == iVMSGUIToolkit::ColumnWidthRole)
			{
				qreal qreal_value = vtValue.toReal();
				qvariant_value = qreal_value;
			}

			if (d->GetRootModel()->setHeaderData(iSection, enumOrientation, qvariant_value, iRole))
            {
                QVector<QAbstractItemModel*> vsSourceModels = d->m_struModelMappingParam.vsSourceModels;
                for (QVector<QAbstractItemModel*>::iterator it = vsSourceModels.begin(); it != vsSourceModels.end(); ++it)
                {
                    QAbstractItemModel* pSourceModel = *it;
                    // 需对正在删除的model做保护
                    if (d->m_struModelMappingParam.pRemModel == pSourceModel)
                    {
                        continue;
                    }

					pSourceModel->setHeaderData(iSection, enumOrientation, qvariant_value, iRole);
                }

                return true;
            }
        }
        else
        {
            Q_ASSERT(NULL);
        }

        return false;
    }

    bool CCombination_Model::insertRows(int iRow, int iCount, const QModelIndex& miParent)
    {
        Q_ASSERT(NULL);
        return QAbstractItemModel::insertRows(iRow, iCount, miParent);
    }

    bool CCombination_Model::insertColumns(int iColumn, int iCount, const QModelIndex& miParent)
    {
        Q_D(const CCombination_Model);

        if (NULL != d->GetRootModel())
        {
            return d->GetRootModel()->insertColumns(iColumn, iCount);
        }
        else
        {
            Q_ASSERT(NULL);
        }

        return false;
    }

    bool CCombination_Model::removeRows(int iRow, int iCount, const QModelIndex& miParent)
    {
        Q_ASSERT(NULL);
        return QAbstractItemModel::removeRows(iRow, iCount, miParent);
    }

    bool CCombination_Model::removeColumns(int iColumn, int iCount, const QModelIndex& miParent)
    {
        Q_D(const CCombination_Model);

        if (NULL != d->GetRootModel())
        {
            return d->GetRootModel()->removeColumns(iColumn, iCount);
        }
        else
        {
            Q_ASSERT(NULL);
        }

        return false;
    }

    Qt::ItemFlags CCombination_Model::flags(const QModelIndex& miProxy) const
    {
        Q_D(const CCombination_Model);
        if (d->IsValidRootIndex(miProxy))
        {
            return d->GetRootModel()->flags(d->GetRootModel()->index(0, 0));
        }
        else
        {
            IndexMap::const_iterator it;
            if (d->ProxyIndexToIterator(miProxy, it))
            {
                // 由于源不一定存在多列，组合创建了多列
                int iTargetColumn = miProxy.column();
                int iTargetRole = iVMSGUIToolkit::CItemFlagRole;
                QModelIndex miMapping = miProxy;
                if (GetMapping(iTargetColumn, iVMSGUIToolkit::CItemFlagRole, iTargetColumn, iTargetRole))
                {
                    miMapping = index(miProxy.row(), iTargetColumn, miProxy.parent());
                }

                CCombination_ModelPrivate::MappingParam* pMappingParam = it.key();
                const QAbstractItemModel* pSourceModel = pMappingParam->pSourceModel;
                if (pSourceModel == d->m_struModelMappingParam.pRemModel)
                {
                    return QAbstractItemModel::flags(miProxy);
                }

                QModelIndex miSourceIndex = d->ProxyToSource(miMapping);
                return pSourceModel->flags(miSourceIndex);
            }
        }

        return QAbstractItemModel::flags(miProxy);
    }

    // 将代理索引转成源索引
    QModelIndex CCombination_Model::MapToSource(const QModelIndex& miProxy) const
    {
        Q_D(const CCombination_Model);
        return d->ProxyToSource(miProxy);
    }

    // 将源索引转成代理索引
    QModelIndex CCombination_Model::MapFromSource(const QModelIndex& sourceIndex) const
    {
        Q_D(const CCombination_Model);
        return d->SourceToProxy(sourceIndex);
    }

    // 通过代理索引获取源model
    QAbstractItemModel* CCombination_Model::GetModelByProxyIndex(const QModelIndex& miProxy)  const
    {
        Q_D(const CCombination_Model);
        return d->GetSourceModelByProxyIndex(miProxy);
    }

    // 根据添加顺序值获取
    QAbstractItemModel* CCombination_Model::GetModel(int iIndex) const
    {
        Q_D(const CCombination_Model);
        if (iIndex < 0 || iIndex >= d->m_struModelMappingParam.vsSourceModels.size())
        {
            return NULL;
        }

        return d->m_struModelMappingParam.vsSourceModels[iIndex];
    }

    // 将pSourceModel移除
    bool CCombination_Model::RemoveModel(QAbstractItemModel* pSourceModel)
    {
        Q_D(CCombination_Model);
        if (pSourceModel == NULL)
        {
            return false;
        }

        // 如果当前的组合源model不存在此model，不允许触发此接口
        int iIndex = d->m_struModelMappingParam.vsSourceModels.indexOf(pSourceModel);
        if (iIndex < 0)
        {
            Q_ASSERT(NULL);
            return false;
        }

        IndexMap::const_iterator it;
        if (d->SourceIndexToIterator(QModelIndex(), pSourceModel, it))
        {
            int iRowCount = it.key()->vsSourceRows.size();
            d->SourceModelAboutToBeRemoved(pSourceModel, QModelIndex(), 0, iRowCount - 1, Qt::Vertical, true);
            //d->SourceModelRemoved(pSourceModel, QModelIndex(), 0, iRowCount - 1, Qt::Vertical);// 此代码不需要触发，由于model并未移出，算起始位置会出错
            //pSourceModel->removeRows(0, pSourceModel->rowCount());
        }

        pSourceModel->disconnect(this);
        d->RemoveFromMapping(QModelIndex(), pSourceModel);
        
        d->m_struModelMappingParam.vsSourceModels.remove(iIndex);
        d->UpdateAllRootMapping(iIndex);
        d->m_struModelMappingParam.UpdateMapping();
        return true;
    }

    // 将pSourceModel销毁,会触发删除消息
    bool CCombination_Model::DeleteModel(QAbstractItemModel* pSourceModel)
    {
        if (RemoveModel(pSourceModel))
        {
            delete pSourceModel;
            pSourceModel = NULL;
            return true;
        }

        return false;
    }

    // 移除所有源model,会触发删除消息
    void CCombination_Model::RemoveAllModels()
    {
        Q_D(CCombination_Model);

        QVector<QAbstractItemModel*>& vsSourceModels = d->m_struModelMappingParam.vsSourceModels;
        for (int i = 0, iCount = vsSourceModels.size(); i < iCount; i++)
        {
            RemoveModel(vsSourceModels[0]);
        }

        Q_ASSERT(vsSourceModels.size() == 0);
    }

    // 销毁所有的源model,会触发删除消息
    void CCombination_Model::DeleteAllModels()
    {
        Q_D(CCombination_Model);

        QVector<QAbstractItemModel*>& vsSourceModels = d->m_struModelMappingParam.vsSourceModels;
        for (int i = 0, iCount = vsSourceModels.size(); i < iCount; i++)
        {
            DeleteModel(vsSourceModels[0]);
        }

        Q_ASSERT(vsSourceModels.size() == 0);
    }

    // 获取所有添加到组合model中的model
    void CCombination_Model::GetAllModels(QVector<QAbstractItemModel*>& vsModels) const
    {
        Q_D(const CCombination_Model);
        QVector<QAbstractItemModel*>& vsSourceModels = d->m_struModelMappingParam.vsSourceModels;
        vsModels.clear();
        for (QVector<QAbstractItemModel*>::const_iterator it = vsSourceModels.constBegin(); it != vsSourceModels.constEnd(); ++it)
        {
            vsModels.append(*it);
        }
    }

    // 获取所有添加到CCombination_Model中的model个数
    int CCombination_Model::GetModelCount() const
    {
        Q_D(const CCombination_Model);
        return d->m_struModelMappingParam.vsSourceModels.size();
    }
}

#include "moc_Combination_Model.cpp"
