#include <iostream>
#include "FieldListView.hh"


FieldListView::FieldListView( QWidget *parent)
    : QWidget( parent )
{

}

void FieldListView::setList(vector<string> *listItems) {

    /*set number of row and column, listview only support 1 column */
    _listItems = listItems;
    nrow = listItems->size();
    ncol = 1;

    /*create QListView */
    m_listView = new QListView(this);
    QStandardItemModel *model = new QStandardItemModel( nrow, 1, this );

    //fill model value
   
    // for each listItem
    int r = 0;
    for (vector<string>::iterator it=listItems->begin(); it!=listItems->end(); ++it) {
        string fieldname = *it;
        QStandardItem *item = new QStandardItem(QString::fromStdString(fieldname));
        model->setItem(r, 0, item);
        r += 1;
    }
    //set model
    m_listView->setModel(model);
    m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //m_listView->setMinimumHeight(listItems->size()*50);
}

void FieldListView::fieldsSelected(bool clicked) {
  QList<QModelIndex> indexList = m_listView->selectionModel()->selectedIndexes(); //  const
  // to get the list of indexes that are selected.
  cout << "indexList size = " << indexList.size() << endl;
  vector<string> *selectedFields = new vector<string>;
  // QModelIndexList is synonym for QList<QModelIndex>.
  for (QList<QModelIndex>::iterator it = indexList.begin(); it!=indexList.end(); ++it) {
    int idx = it->row();
    cout << idx << "selected " << _listItems->at(idx) << endl;
    selectedFields->push_back(_listItems->at(idx));
  }

  
  // TODO: send a ??? to the PolarManager???
  emit sendSelectedFieldsForImport(selectedFields);
}