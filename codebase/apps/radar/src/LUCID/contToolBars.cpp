#include "contToolBars.h"

contToolBars::contToolBars()
{
    //products toolbar
    //products = this->addToolBar(tr("productsToolbar"));
    //addToolBar(Qt::RightToolBarArea, products);
    products = new QToolBar;
    QString allProducts = "All Products";
    products->addAction(allProducts);
    for (int i=1; i<30; i++)
    {
        QString nameOfAction = "Product: " + QString::number(i);
        products ->addAction(nameOfAction);
    }

    //maps toolbar
    //maps = this->addToolBar(tr("mapOverlaysToobar"));
    //addToolBar(Qt::RightToolBarArea, maps);
    maps = new QToolBar;
    QString allMaps = "All Maps";
    maps->addAction(allMaps);
    for (int i=1; i<30; i++)
    {
        QString nameOfAction = "Map: " + QString::number(i);
        maps->addAction(nameOfAction);
    }
}


contToolBars::~contToolBars()
{
    delete products;
    delete maps;
}













