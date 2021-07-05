package org.jetbrains.jwm.examples;

import org.jetbrains.jwm.*;
import org.jetbrains.skija.*;

public interface SkijaLayer extends Layer {
    Canvas beforePaint();
    void afterPaint();
}