#ifndef VIEWOVERLAYSDIALOG_H
#define VIEWOVERLAYSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

class viewOverlaysDialog : public QDialog
{
    Q_OBJECT
public:
    explicit viewOverlaysDialog(QWidget *parent = nullptr);
    ~viewOverlaysDialog();

    QLabel *lineLayerLabel, *activeLabel, *fromLabel, *toLabel, *byLabel,
    *showLegendLabel, *lineLabelsLabel, *labelSizeLabel, *mapsLabel,
    *rangeRingLabel, *landUseLabel, *brightnessLabel, *terrainMaskingLabel;
    QComboBox *lineLayerSelector, *fieldLayerSelector, *colorSelector;
    QCheckBox *activeCheckBox, *showLegendCheck, *lineLabelsCheckBox,
    *rangeRingCheckBox, *landUseCheckBox, *terrainMaskingCheck;
    QLineEdit *fromInput, *toInput, *byInput;
    QSlider *labelSizeSlider, *brightnessSlider;
    QPushButton *changeColorButton, *rangeRingColors, *savePlanViewButton;
    QListWidget *mapsList;
    QFrame *line1, *line2, *line3;
    QHBoxLayout *overLayoutH1, *overLayoutH2, *overLayoutH3, *overLayoutH4,
    *overLayoutH5, *overLayoutH6, *overLayoutH7, *overLayoutH8;
    QVBoxLayout *overLayoutV1;
};

#endif // VIEWOVERLAYSDIALOG_H
