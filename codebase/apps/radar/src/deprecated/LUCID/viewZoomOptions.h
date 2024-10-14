#ifndef VIEWZOOMOPTIONS_H
#define VIEWZOOMOPTIONS_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>

class viewZoomOptions : public QDialog
{
    Q_OBJECT
public:
    explicit viewZoomOptions(QWidget *parent = nullptr);
    ~viewZoomOptions();

signals:

public slots:

private:
    QDialog *zoomOptions;
    QPushButton *zoom10, *zoom100, *zoom1000, *zoomSaved, *zoomSaved2, *zoomReset;
    QVBoxLayout *zoomLayout;

};

#endif // VIEWZOOMOPTIONS_H
