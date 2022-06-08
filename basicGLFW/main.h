//
// Created by tztz8 on 6/2/22.
//

#ifndef OPENGL_STARTING_PLACE_BUNNY_MAIN_H
#define OPENGL_STARTING_PLACE_BUNNY_MAIN_H

extern bool devILIsSetup;
extern int screenHeight;
extern int screenWidth;

//          --- Pre-Def Methods ---

/**
 * Set the window flag to exit window
 */
void tellWindowToClose();

/**
 * check if key was press <br>
 * will only return true ones intel key not press on next check <br>
 * usefully for toggle things like show lines flag
 * @param key char often the key to store the state
 * @param GLFW_key the key that glfw recognise
 * @return false if still being press or not being press
 */
bool checkKey(char key, int GLFW_key);

#endif //OPENGL_STARTING_PLACE_BUNNY_MAIN_H
