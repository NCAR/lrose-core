#ifndef FIELDLISTVIEW_HH
#define FIELDLISTVIEW_HH

#include <string>
#include <vector>
#include <QListView>
#include <QStandardItemModel>
#include <QApplication>

using namespace std;

class FieldListView : public QWidget
{
	Q_OBJECT

public:
        FieldListView( QWidget *parent=0);
        void setList(vector<string> *listItems);

public slots:
  void fieldsSelected(bool clicked);

signals:
  void sendSelectedFieldsForImport(vector<string> *selectedFields);

private:
        int nrow, ncol;
        QListView *m_listView;
        vector<string> *_listItems;
};

#endif