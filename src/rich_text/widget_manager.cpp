#include "widget_manager.h"

using namespace AB;

namespace RichText {
    WidgetId WidgetManager::widget_id = 0;
    WidgetManager::WidgetManager(const File& file): m_file(file) {

    }
}
