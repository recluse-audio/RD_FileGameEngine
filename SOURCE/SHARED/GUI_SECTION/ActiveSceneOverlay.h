/**
 * Made by Ryan Devens on 2026-03-12
 */

#pragma once

class GraphicsRenderer;

/**
 * Abstract base for overlays rendered on top of an active scene's content.
 *
 * Concrete subclasses:
 *   PasswordEntryOverlay  — locks the scene behind a character-scroll password UI
 *   MultipleChoiceOverlay — presents four labelled answer buttons
 *
 * ActiveSceneSection owns one of these via unique_ptr, selecting the appropriate
 * subclass in setActiveScene() based on the LevelScene data.
 */
class ActiveSceneOverlay
{
public:
    virtual ~ActiveSceneOverlay() = default;

    virtual void draw(GraphicsRenderer& renderer) const = 0;
    virtual void registerHit(int x, int y) = 0;
};
