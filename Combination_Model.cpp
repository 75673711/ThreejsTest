/*
 * @file  Combination_Model.cpp
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
        Q_DECLARE_PUBLIC(CCombination_Model)// ��CCombinationModelPrivate��ͨ��q_func()���Ի��CCombinationModel��ָ��q_ptr

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
            QAbstractItemModel* pSourceModel; // Դmodel
            QVector<int> vsSourceRows;    // Դ��
            QVector<int> vsSourceColumns; // Դ��
            int iProxyRowBegin;           // ��¼����model����ʼλ��
            QVector<int> vsProxyRows;     // Դ�е�vector����
            QVector<int> vsProxyColumns;  // Դ�е�vector����
            QVector<QModelIndex> vsMappedChildrens;
            QHash<MappingParam*, QModelIndex>::const_iterator iterMap; // ��¼��ǰ�ṹ�����ݵĵ�����
        };

        struct ModelMappingParam 
        {
        public:
            ModelMappingParam()
                : pRemModel(NULL)
            {

            }

        public:
            // ��������ӳ���ϵ
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
            QVector<int> vsSources;   // Դ
            QVector<int> vsProxys;    // ����
            QVector<QAbstractItemModel*> vsSourceModels; // ������ӵ����CCombinationModel�е�models
            QAbstractItemModel* pRemModel; // ���model���ⲿɾ������¼
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
        // ����2ΪԴmodel�����ӽڵ��������������root model��Ӧ�Ľڵ�
        // ����1Ϊ����2���ڵĽڵ�����
        mutable QHash<MappingParam*, QModelIndex> m_hashSourceIndexMappings;

    private:
        QAbstractItemModel* m_pRootModel; // root model
        bool m_bRemRootModel;
        mutable QAbstractItemModel* m_pDefaultRootModel;

    public:
        // ��ȡroot model
        QAbstractItemModel* GetRootModel() const;
        // ͨ��������ȡԴmodel
        QAbstractItemModel* GetSourceModelByProxyIndex(const QModelIndex& miProxy) const;
        // ͨ����ϵĵ�һ���ڵ���кŻ�ȡԴ
        QAbstractItemModel* GetSourceModelByProxyRow(int iProxyRow) const;
        
        // ��ȡԴmodel��iSourceRow�����model�е�λ��(ֻ���ǵ�һ�ڵ�) ����ָpSourceModel����
        int GetSourceRowToProxyRow(const QAbstractItemModel* pSourceModel, int iSourceRow) const;
        // ͨ��modelλ�û�ȡ�к�
        void GetSourceModelToProxyRow(int iSourceModelIndex, int& iProxyStart, int& iProxyEnd) const;
    public:
        // ���������е�root�ڵ�(����ӻ�ɾ��ʱ)
        void UpdateAllRootMapping(int iModelIndex);
        // ����pSourceModel�Ľڵ����
        void UpdateModelMapping(QAbstractItemModel* pSourceModel, const QModelIndex& miSourceParent = QModelIndex());
        // ���¸��ڵ��ӳ��
        void UpdateRootMappingProxyRowBegin();

    public:
        // �Ƿ�Ϊ��Ч�ĸ��ڵ�
        bool IsValidRootIndex(const QModelIndex& miProxy) const
        {
            QAbstractItemModel* pRootModel = GetRootModel();
            return ((pRootModel != NULL) && (pRootModel->hasChildren()) && (miProxy.isValid() && !miProxy.parent().isValid()));
        }

        // �Ƿ�Ϊ��Ч�ĸ��ĸ��ڵ�
        bool IsValidRootParentIndex(const QModelIndex& miProxyParent) const
        {
            QAbstractItemModel* pRootModel = GetRootModel();
            return ((pRootModel != NULL) && (pRootModel->hasChildren()) && !miProxyParent.isValid());
        }

    public:
        // ����ӳ���ϵ
        bool CreateMapping(const QModelIndex& miSourceParent, const QAbstractItemModel* pSourceModel, QHash<MappingParam*, QModelIndex>::const_iterator& it) const;
        // ����������ת��Դ����
        QModelIndex ProxyToSource(const QModelIndex& miProxy, bool bFilterRoot = false) const;
        // ��Դ����ת�ɴ�������
        QModelIndex SourceToProxy(const QModelIndex& miSource) const;
        // ɾ��ӳ��
        void RemoveFromMapping(const QModelIndex& miSourceParent, const QAbstractItemModel* pSourceModel);
        // ȡ��ӳ��
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
        // ����model����֮��Ͳ��ܵ��������нӿڣ������뷢�����ź�
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
        // ���Դmodel�����ݹ�����Ϣ
        for (IndexMap::Iterator it = m_hashSourceIndexMappings.begin(); it != m_hashSourceIndexMappings.end(); )
        {
            MappingParam* pMapping = it.key();
            it = m_hashSourceIndexMappings.erase(it);
            delete pMapping;
        }

        m_hashSourceIndexMappings.clear();

        // �������model�������������ٲ�������ֹ����ɾ��
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

    // ʹ�ô˽ӿ������ж��Ƿ���ڣ�����ȷ�ϵ�һ�δ���
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

        // ��¼Դmodel��
        int iSourceRows = pSourceModel->rowCount(miSourceParent);
        pMappingParam->vsSourceRows.reserve(iSourceRows);
        for (int i = 0; i < iSourceRows; ++i) 
        {
            pMappingParam->vsSourceRows.append(i);
        }

        // ��¼Դmodel��
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
        if (pSourceModel != GetRootModel() && !miSourceParent.isValid()) // �����Ч,˵����root�ڵ�
        {
            iRow = GetSourceRowToProxyRow(pSourceModel, 0);
        }

        pMappingParam->iProxyRowBegin = iRow;
        // ��¼����model����
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

    // ��ȡroot model
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

    // ��ȡCCombinationModel�е�����miCur�ڱ����model
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
        QVector<int> vsSources = m_struModelMappingParam.vsSources;   // Դ
        QVector<int> vsProxys = m_struModelMappingParam.vsProxys;     // ����
        QVector<QAbstractItemModel*> vsSourceModels = m_struModelMappingParam.vsSourceModels;
        for (QVector<int>::const_iterator it = vsProxys.constBegin(); it != vsProxys.constEnd(); ++it)
        {
            // ���ҵ�Դmodel
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

            // �����ҵ�ӳ������
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

    // ��ȡԴmodel��row�����model�е�λ��(ֻ���ǵ�һ�ڵ�)
    int CCombination_ModelPrivate::GetSourceRowToProxyRow(const QAbstractItemModel* pSourceModel, int iSourceRow) const
    {
        QVector<QAbstractItemModel*> vsSourceModels = m_struModelMappingParam.vsSourceModels;
        int iRow = 0;
        // ����m_vsModels�������������������ӵ�model
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

    // ͨ��modelλ�û�ȡ�к�
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

    // ���������е�root�ڵ�(����ӻ�ɾ��ʱ)
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

            // ��¼Դmodel��
            int iSourceRowCount = pSourceModel->rowCount();
            pMappingParam->vsSourceRows.clear();
            for (int i = 0; i < iSourceRowCount; ++i) 
            {
                pMappingParam->vsSourceRows.append(i);
            }

            // ��¼Դmodel��
            int iSourceColCount = pSourceModel->columnCount();
            pMappingParam->vsSourceColumns.clear();
            for (int i = 0; i < iSourceColCount; ++i) 
            {
                pMappingParam->vsSourceColumns.append(i);
            }
            pMappingParam->iProxyRowBegin = iProxyRowBegin;
            // ��¼����model����
            pMappingParam->vsProxyRows.resize(iSourceRowCount);
            BuildSourceToProxyMapping(pMappingParam->vsSourceRows, pMappingParam->vsProxyRows);
            pMappingParam->vsProxyColumns.resize(iSourceColCount);
            BuildSourceToProxyMapping(pMappingParam->vsSourceColumns, pMappingParam->vsProxyColumns);
        }
    }

    // ����pSourceModel�Ľڵ����
    void CCombination_ModelPrivate::UpdateModelMapping(QAbstractItemModel* pSourceModel, const QModelIndex& miSourceParent)
    {
        Q_Q(const CCombination_Model);
        int iRowCount = pSourceModel->rowCount(miSourceParent);
        if (iRowCount > 0)
        {
            // �ȴ���������
            QHash<MappingParam*, QModelIndex>::const_iterator itMap;
            if (!SourceIndexToIterator(miSourceParent, pSourceModel, itMap))
            {
                CreateMapping(miSourceParent, pSourceModel, itMap);
            }
            // �����ӽڵ�Ĺ���
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

    // ����������ת��Դ����
    QModelIndex CCombination_ModelPrivate::ProxyToSource(const QModelIndex& miProxy, bool bFilterRoot) const
    {
        if (!miProxy.isValid())
        {
            return QModelIndex();
        }

        if (miProxy.model() != q_func()) 
        {
            qWarning() << "CCombinationModel: index from wrong model passed to ProxyToSource";
            // �˶��Բ��ܴ��ڣ�����model�ٴ�����modelʱ�ᴥ����ȡ�����ӿڣ��ᱨ����
            // Q_ASSERT(!"CCombinationModel: index from wrong model passed to ProxyToSource");
            return QModelIndex();
        }

        // ���ڵ�
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

        // ���������ɾ����������Ч
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
            CreateMapping(miSourceParent, pSourceModel, it); // �ȴ����յ�
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

        // ���ͼ���������Ϣ
        if (enumOrient == Qt::Vertical)// vsSources[] vsProxys[-1]
        {
            q->beginInsertRows(miProxyParent, iStart + pMappingParam->iProxyRowBegin, iEnd + pMappingParam->iProxyRowBegin);
        }
        else
        {
            q->beginInsertColumns(miProxyParent, iStart, iEnd);
        }

        // �ı�Դmodel����ֵ
        for (int i = iStart; i <= iEnd; ++i)
        {
            vsSources.insert(i, i);
        }

        // ���ù���
        BuildSourceToProxyMapping(vsSources, vsProxys);

        if (!miSourceParent.isValid())
        {
            UpdateRootMappingProxyRowBegin();
        }

        // ����ͬ���󣬷��������Ϣ
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
        // ɾ�������е�����
        vsProxys.remove(iSourceStart, iDeltaItemCount);

        int iSouceCount = vsSources.size();
        if (iSouceCount != vsProxys.size())
        {
            Q_ASSERT(NULL);
        }

        // ���¼���Դ����
        for (int iSource = 0; iSource < iSouceCount; ++iSource)
        {
            int iSourceRow = vsSources.at(iSource);
            if (iSourceRow >= iSourceStart) 
            {
                Q_ASSERT(iSourceRow - iDeltaItemCount >= 0);
                vsSources.replace(iSource, iSourceRow - iDeltaItemCount);
            }
        }
        // ���¹���
        BuildSourceToProxyMapping(vsSources, vsProxys);
        // ������child�еĹ���
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

        // ���ͼ���������Ϣ
        if (enumOrient == Qt::Vertical)// vsSources[] vsProxys[-1]
        {
            q->beginInsertRows(miProxyParent, iStart + pMappingParam->iProxyRowBegin, iEnd + pMappingParam->iProxyRowBegin);
        }
        else
        {
            q->beginInsertColumns(miProxyParent, iStart, iEnd);
        }

        // �ı�Դmodel����ֵ
        for (int i = iStart; i <= iEnd; ++i)
        {
            vsSources.insert(i, i);
        }

        // ���ù���
        BuildSourceToProxyMapping(vsSources, vsProxys);

        // ����ͬ���󣬷��������Ϣ
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

        // ���������ɾ����������Ч
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

                const QModelIndex miProxyBottomRight = CreateIndex(iProxyEndRow, q->columnCount() - 1 // ��Ϊ�п��ܴ���ӳ�䣬�������ݸ��·�Χ��������
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

    // ������������(������������Դmodel�����иı䣬��������)
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

    // ����root�ڵ�,����model���ӵ��˽ڵ���
    void CCombination_Model::SetRootModel(QAbstractItemModel* pRootModel)
    {
        Q_D(CCombination_Model);
        Q_ASSERT(pRootModel);
        if (d->m_struModelMappingParam.vsSourceModels.size() > 0)
        {
            Q_ASSERT(NULL);
            return;
        }

        // ���Ĭ�ϴ��ڣ�˵���Ѿ�����Ĭ��
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

    // ��ȡroot�ڵ�model
    QAbstractItemModel* CCombination_Model::GetRootModel() const
    {
        Q_D(const CCombination_Model);
        return d->GetRootModel();
    }

    // ����root���źŲ�
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
        // ���±���ӽ����ӵĽڵ����
        d->UpdateModelMapping(pSourceModel);
        if (iSourceRowCount > 0)
        {
            endInsertRows();
        }
        return true;
    }

    // ��Դmodel���˳�����λ��iIndex����
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
//         // �ȴ����յ�ӳ��
//         for (QList<QAbstractItemModel*>::iterator itModel = listSourceModels.begin(); itModel != listSourceModels.end(); ++itModel)
//         {
//             CCombination_ModelPrivate::MappingParam* pMappingParam = new CCombination_ModelPrivate::MappingParam;
//             pMappingParam->pSourceModel = *itModel;
// 
//             // ��¼Դmodel��
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
//             // ���±���ӽ����ӵĽڵ����
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
            // ���m_pRootMappingParam��������Ч����Ҫɾ����ȡ��Ч��
            if (NULL != d->m_pRootMappingParam && d->m_pRootMappingParam->ProxyRowToSourceRow(iRow) == -1)
            {
                return QModelIndex();
            }

            // �����ھʹ���
            d->CreateRootMapping();

            return QAbstractItemModel::createIndex(iRow, iColumn);
        }
        else
        {
            QModelIndex miSourceParent = d->ProxyToSource(miProxyParent, true);
            // �Ȳ鿴�Ƿ���ڣ���ֹ����ɾ��ͨ��GetSourceModelByRow��ȡ�����ݲ��ԣ�
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
            iRowCount = qMin(1, iRowCount);// ��ʱֻ֧��һ��
        }
        else if (d->IsValidRootIndex(miParent))
        {
            // �����Ч,˵����root�ڵ�,��������е�Դmodel�Ľڵ���
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
                // �����Ч,˵����root�ڵ�,��������е�Դmodel�Ľڵ���
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

        // ��ʱĬ��1
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
        {   // ����root�ڵ�ĸ��ڵ�
            if (d->GetRootModel()->hasChildren())
            {
                return true;
            }
        }
        else if (d->IsValidRootIndex(miProxyParent))
        {
            // ����root�ڵ�
            // ��������е�Դmodel�Ľڵ���
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
                // ����Դ
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
                // �����Ч,˵����root�ڵ�,��������е�Դmodel�Ľڵ���
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
            // �����ڴ�CFilterStringListRoleֵ�����⴦��������ɫ�����˶�λ��ʹ�ô˹��ܣ�
            QVariant vt = headerData(0, Qt::Horizontal, CFilterStringListRole);
            if (vt.isValid())
            {
                QString strDisplay = miProxy.data(Qt::DisplayRole).toString();

                // ƥ���ַ�
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
			//��ͷ�п�ƥ��4K��
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
                    // �������ɾ����model������
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
                // ����Դ��һ�����ڶ��У���ϴ����˶���
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

    // ����������ת��Դ����
    QModelIndex CCombination_Model::MapToSource(const QModelIndex& miProxy) const
    {
        Q_D(const CCombination_Model);
        return d->ProxyToSource(miProxy);
    }

    // ��Դ����ת�ɴ�������
    QModelIndex CCombination_Model::MapFromSource(const QModelIndex& sourceIndex) const
    {
        Q_D(const CCombination_Model);
        return d->SourceToProxy(sourceIndex);
    }

    // ͨ������������ȡԴmodel
    QAbstractItemModel* CCombination_Model::GetModelByProxyIndex(const QModelIndex& miProxy)  const
    {
        Q_D(const CCombination_Model);
        return d->GetSourceModelByProxyIndex(miProxy);
    }

    // �������˳��ֵ��ȡ
    QAbstractItemModel* CCombination_Model::GetModel(int iIndex) const
    {
        Q_D(const CCombination_Model);
        if (iIndex < 0 || iIndex >= d->m_struModelMappingParam.vsSourceModels.size())
        {
            return NULL;
        }

        return d->m_struModelMappingParam.vsSourceModels[iIndex];
    }

    // ��pSourceModel�Ƴ�
    bool CCombination_Model::RemoveModel(QAbstractItemModel* pSourceModel)
    {
        Q_D(CCombination_Model);
        if (pSourceModel == NULL)
        {
            return false;
        }

        // �����ǰ�����Դmodel�����ڴ�model�����������˽ӿ�
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
            //d->SourceModelRemoved(pSourceModel, QModelIndex(), 0, iRowCount - 1, Qt::Vertical);// �˴��벻��Ҫ����������model��δ�Ƴ�������ʼλ�û����
            //pSourceModel->removeRows(0, pSourceModel->rowCount());
        }

        pSourceModel->disconnect(this);
        d->RemoveFromMapping(QModelIndex(), pSourceModel);
        
        d->m_struModelMappingParam.vsSourceModels.remove(iIndex);
        d->UpdateAllRootMapping(iIndex);
        d->m_struModelMappingParam.UpdateMapping();
        return true;
    }

    // ��pSourceModel����,�ᴥ��ɾ����Ϣ
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

    // �Ƴ�����Դmodel,�ᴥ��ɾ����Ϣ
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

    // �������е�Դmodel,�ᴥ��ɾ����Ϣ
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

    // ��ȡ������ӵ����model�е�model
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

    // ��ȡ������ӵ�CCombination_Model�е�model����
    int CCombination_Model::GetModelCount() const
    {
        Q_D(const CCombination_Model);
        return d->m_struModelMappingParam.vsSourceModels.size();
    }
}

#include "moc_Combination_Model.cpp"
