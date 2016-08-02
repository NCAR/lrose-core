
//////////////////////////////////////////////////////////////
//
// Header file for VkTabbedDeck
//
//    This class is a ViewKit component, which should eventually
// be added to the ViewKit library.
// Normally, nothing in this file should need to be changed.
//
//////////////////////////////////////////////////////////////
#ifndef VKTABBEDDECK_H
#define VKTABBEDDECK_H
#include <Vk/VkComponent.h>

class VkTabbedDeck: public VkComponent { 

  public:

    VkTabbedDeck(const char *, Widget );
    ~VkTabbedDeck();
    const char *className();

    void registerChild ( VkComponent*, const char *name);
    void registerChild ( Widget, const char *name);

    Widget deckParent();

    static const char *const tabPopupCallback;    

  protected:

    void pop ( VkCallbackObject *, void *, void *);
    
    class VkDeck *_vkdeck;
    class VkTabPanel *_tabs;
};
#endif
