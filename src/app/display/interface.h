/**
 * Display Drawer Interface - Defines drawing methods for different display states
 * Allows for consistent drawing implementations that can be swapped
 */
#include <U8g2lib.h>
class DisplayDrawer {
public:
    virtual ~DisplayDrawer() = default;

    /**
     * Draw the display content
     * @param display U8G2 display instance
     * @param manager DisplayManager for accessing shared data
     */
    virtual void draw() = 0;
};