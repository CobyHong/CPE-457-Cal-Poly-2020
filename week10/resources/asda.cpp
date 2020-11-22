if (FirstTime) {
      assert(GLTextureWriter::WriteImage(texBuf[0],"Texture_output.png"));
      FirstTime = 0;
    }
    int inp = 0;
    int frame = 1;
    for (int i = 0; i < 3; i++) {

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[frame]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ProcessDrawTex(texBuf[inp]);

        if (inp == 0) {
            inp = 1;
        }
        else {
            inp = 0;
        }
        if (frame == 0) {
            frame = 1;
        }
        else {
            frame = 0;
        }
    }

    //regardless NOW set up to render to the screen = 0
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /* now draw the actual output  to the default framebuffer - ie display / 
    / note the current base code is just using one FBO and texture - will need
      to change that  - we pass in texBuf[0] right now */
    ProcessDrawTex(texBuf[0]);





//after  the assertion statement in the code

//bind the framebuffer to the second one, the one with your horizontal shift
// this saves the results of your first FBO to be processed later
glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[1]);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// process the 2D rendering of the original 3D scene through your first frame buffer (vertical shift)
ProcessDrawTex(texBuf[0], tex_prog);

//regardless NOW set up to render to the screen = 0
glBindFramebuffer(GL_FRAMEBUFFER, 0);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// Draw the processed texture with the 2nd FBO with horizontal shift
ProcessDrawTex(texBuf[1], tex_prog2);