#ifndef VIEWGRIDCONFIGDIALOG_H
#define VIEWGRIDCONFIGDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPushButton>

class viewGridConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit viewGridConfigDialog(QWidget *parent = nullptr);
    ~viewGridConfigDialog();


    QDialog *gridDataLayers;
    QLabel  *gridLayerLabel, *gridLabel, *gridUrlLabel, *gridTopBotLabel, *gridLegendLabel, *gridMinValueLabel,
            *gridMaxValueLabel, *gridDeltaLabel, *gridTimeSlopLabel, *gridTimeOffsetLabel, *gridAltOffsetLabel,
            *gridAutoUpdateLabel, *gridRequestCompositeLabel, *gridAutoscaleLabel, *gridColorScale, *gridUnits,
            *gridRenderAsLabel;
    QComboBox *gridLayerSelector, *gridSelector, *gridTopBotSelector, *gridRenderAsSelector;
    QLineEdit *gridUrlInput, *gridMinValueInput, *gridMaxValueInput, *gridDeltaInput, *gridTimeSlopInput,
            *gridTimeOffsetInput, *gridAltOffsetInput, *gridColorMapInput;
    QCheckBox *gridLegendBox, *gridAutoUpdateBox, *gridRequestCompositeBox, *gridAutoscaleBox;
    QPushButton *gridColorMap;

signals:

public slots:

private:

    QHBoxLayout *gridWindowLayoutH1, *gridWindowLayoutH2, *gridWindowLayoutH3, *gridWindowLayoutH4,
            *gridWindowLayoutH5, *gridWindowLayoutH6, *gridWindowLayoutH7, *gridWindowLayoutH8,
            *gridWindowLayoutH9, *gridWindowLayoutH10, *gridWindowLayoutH11, *gridWindowLayoutH12,
            *gridWindowLayoutH13, *gridWindowLayoutH14, *gridWindowLayoutH15, *gridWindowLayoutH16;
    QVBoxLayout *gridWindowLayoutV1, *gridWindowLayoutV2, *gridWindowLayoutV3, *gridWindowLayoutV4, *gridWindowLayoutVAll;


protected:



};

#endif // VIEWGRIDCONFIGDIALOG_H







