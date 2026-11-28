#ifndef WINDDIALOGVIEW_H
#define WINDDIALOGVIEW_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QSlider>
#include <QSlider>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

class viewWindDialog : public QDialog
{
    Q_OBJECT
public:
    explicit viewWindDialog(QWidget *parent = nullptr);
    ~viewWindDialog();

    QComboBox *windSelector, *windColorSelect, *windStyles;

signals:

public slots:

private:
    QLabel *windSelectLabel, *windUrlLabel, *windColorSelectText, *windNumLabel, *windWidthLabel, *windLengthLabel,
            *UNameLabel, *VNameLabel, *WNameLabel,*windTimeSlopLabel, *windTimeOffsetLabel, *windAltitudeOffsetLabel,
            *windStylesLabel, *windLegendLabel;
    QLineEdit *windUrlInput, *UNameInput, *VNameInput, *WNameInput, *windTimeSlopInput, *windTimeOffsetInput,
            *windAltitudeOffsetInput;
    QSlider *windNumSlider, *windWidthSlider, *windLengthSlider;
    QCheckBox *windLegendSelect;
    QHBoxLayout *windLayoutH1, *windLayoutH2, *windLayoutH3;
    QVBoxLayout *windLayoutV1, *windLayoutV2, *windLayoutV3, *windLayoutV4, *windLayoutVAll;

};

#endif // WINDDIALOG_H
