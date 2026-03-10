#include "PasswordEntryComponent.h"

std::string PasswordEntryComponent::getEntryString() const
{
    std::string result;
    result.reserve(mBoxes.size());
    for (const auto& box : mBoxes)
        result += box.getValue();
    return result;
}

bool PasswordEntryComponent::registerHit(int x, int y)
{
    for (auto& box : mBoxes)
        if (box.registerHit(x, y))
            return true;
    return false;
}

void PasswordEntryComponent::draw(GraphicsRenderer& renderer) const
{
    for (const auto& box : mBoxes)
        box.draw(renderer);
}
