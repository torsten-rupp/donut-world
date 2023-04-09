#include <string.h>
#include <stdio.h>
#include <math.h>

#include <vector>
#include <array>
#include <random>
#include <functional>
#include <thread>

#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/quaternion.hpp>

#include "color.h"
#include "mapGenerator.h"
#include "islands.h"

#define TEXTURE_TYPE_FIXED     1
#define TEXTURE_TYPE_GENERATED 2
//#define TEXTURE_TYPE TEXTURE_TYPE_FIXED
#define TEXTURE_TYPE TEXTURE_TYPE_GENERATED


#define LOCAL static

#if (TEXTURE_TYPE == TEXTURE_TYPE_GENERATED)
  // map texture
  const uint TEXTURE_WIDTH  = 512;
  const uint TEXTURE_HEIGHT = 512;
#else
  #include "worldmap6.h"
#endif

// OpenGL view size
const uint VIEW_WIDTH  = 800;
const uint VIEW_HEIGHT = 600;

// donut faces+size
const uint STEPS1 = 18 * 2;
const uint STEPS2 = 24 * 2;
const float R1 = 0.3;
const float R2 = 0.6;

typedef struct
{
  glm::vec3 position;
  glm::vec3 color;
  glm::vec2 uv;
} Vertex;

LOCAL const char *VERTEX_SHADER =
  "#version 300 es\n"
  "uniform mat4 projectionViewModel;\n"
  "in vec3      vPosition;\n"
  "in vec3      vColor;\n"
  "in vec2      vUV;\n"
  "out vec3     color;\n"
  "out vec2     uv;\n"
  "\n"
  "void main()\n"
  "{\n"
  "  gl_Position = projectionViewModel * vec4(vPosition, 1.0);\n"
  "  color = vColor;\n"
  "  uv = vUV;\n"
  "}\n";

LOCAL const char *FRAGMENT_SHADER =
  "#version 300 es\n"
  "precision mediump float;\n"
  "uniform sampler2D textureSampler;\n"
  "uniform vec2      textureTranslate;\n"
  "in vec3           color;\n"
  "in vec2           uv;\n"
  "out vec4          fragment;\n"
  "void main()\n"
  "{\n"
  "  vec2 w = vec2(uv.x+textureTranslate.x,uv.y+textureTranslate.y);\n"
  "  fragment = texture2D(textureSampler, w);\n"
  "}\n";


#if (TEXTURE_TYPE == TEXTURE_TYPE_GENERATED)
  #if 0
    LOCAL std::array<Color, TEXTURE_WIDTH *TEXTURE_HEIGHT> textureData;
  #else
    LOCAL std::array<std::array<Color, TEXTURE_WIDTH>, TEXTURE_HEIGHT> textureData;
  #endif
#endif

LOCAL GtkWidget          *area;
LOCAL GDateTime          *startDateTime = g_date_time_new_now_local();
LOCAL GDateTime          *lastDateTime;

LOCAL std::vector<Vertex> vertices;

LOCAL GLuint              vertexBuffer;
LOCAL GLuint              fragmentShader, vertexShader;
LOCAL GLuint              program;
LOCAL GLuint              vertexArray;

LOCAL GLint               projectionViewModelLocation;
LOCAL GLint               textureTranslateLocation;
LOCAL glm::vec2           textureTranslate = { 0.0, 0.0 };

LOCAL glm::mat4           model;

LOCAL Map                 map(TEXTURE_WIDTH, TEXTURE_HEIGHT);
LOCAL bool                newMapFlag = FALSE;

LOCAL GtkWidget           *buttonNewMap;
LOCAL GtkWidget           *buttonFindIslands;
LOCAL GtkWidget           *islandsText;
LOCAL GtkWidget           *statusBar;

// ---------------------------------------------------------------------

/** add triangle to vertices
 * @param vertices vertices
 * @param p0,p1,p2 triangle points
 * @param uv0,uv1,uv2 triangle UV map points
 */
LOCAL void addTriangle(std::vector<Vertex> &vertices,
                       const glm::vec3     &p0,
                       const glm::vec3     &p1,
                       const glm::vec3     &p2,
                       const glm::vec2     &uv0,
                       const glm::vec2     &uv1,
                       const glm::vec2     &uv2
                      )
{
  glm::vec3 c0 = { 1.0f, 0.0f, 0.0f };
  glm::vec3 c1 = { 0.0f, 1.0f, 0.0f };
  glm::vec3 c2 = { 0.0f, 0.0f, 1.0f };

  // add triangle
  vertices.push_back(Vertex{p0, c0, uv0});
  vertices.push_back(Vertex{p1, c1, uv1});
  vertices.push_back(Vertex{p2, c2, uv2});
}

/** add quad to vertices
 * @param vertices vertices
 * @param p0,p1,p2,p3 quad points
 * @param uv0,uv1,uv2,uv3 quad UV map points
 */
LOCAL void addQuad(std::vector<Vertex> &vertices,
                   const glm::vec3     &p0,
                   const glm::vec3     &p1,
                   const glm::vec3     &p2,
                   const glm::vec3     &p3,
                   const glm::vec2     &uv0,
                   const glm::vec2     &uv1,
                   const glm::vec2     &uv2,
                   const glm::vec2     &uv3
                  )
{
  addTriangle(vertices, p0, p1, p2, uv0, uv1, uv2);
  addTriangle(vertices, p2, p3, p0, uv2, uv3, uv0);
}

#if 0
LOCAL void createCube(std::vector<Vertex> &vertices)
{
  glm::vec2 uv{0.0f, 0.0f};

  addQuad(vertices,
          glm::vec3(-1, -1, -1),
          glm::vec3(1, -1, -1),
          glm::vec3(1, 1, -1),
          glm::vec3(-1, 1, -1),
          uv, uv, uv, uv
         );

  addQuad(vertices,
          glm::vec3(-1, -1, 1),
          glm::vec3(1, -1, 1),
          glm::vec3(1, 1, 1),
          glm::vec3(-1, 1, 1),
          uv, uv, uv, uv
         );
}
#endif

/** create donut world segment
 * @param segment segment
 * @param maxSegmentCount max. segment count
 * @param radius radius
 * @param translateY translation along y-axis
 */
LOCAL void createDonutWorldSegment(glm::vec3 segment[], uint maxSegmentCount, float radius, float translateY)
{
  // get circle segment
  segment[0] = glm::vec3(0.0f, radius, 0.0f);

  for (uint i = 1; i < maxSegmentCount; i++)
  {
    float th = (2 * M_PI * (float)i) / (float)maxSegmentCount;

    glm::vec3 eulers(th, 0.0, 0.0);
    glm::quat q = glm::quat(eulers);

    segment[i] = segment[0] * q;
  }

  segment[maxSegmentCount - 1] = segment[0];

  // translate along y-axis
  for (uint i = 0; i < maxSegmentCount; i++)
  {
    segment[i] += glm::vec3(0.0f, translateY, 0.0f);
  }
}

/** create donut world
 * @param vertices vertices
 */
LOCAL void createDonutWorld(std::vector<Vertex> &vertices)
{
  glm::vec3 segment [STEPS1];
  glm::vec3 segment0[STEPS1];
  glm::vec3 segment1[STEPS1];

  createDonutWorldSegment(segment, STEPS1, R1, R2);

  glm::vec2 uv[4] {glm::vec2(0.0f, 0.0f),
                   glm::vec2(0.0f, 0.0f),
                   glm::vec2(0.0f, 0.0f),
                   glm::vec2(0.0f, 0.0f)
                  };
  memcpy(segment0, segment, sizeof(segment0));

  for (uint i = 1; i < STEPS2; i++)
  {
    float th = (2 * M_PI * (float)i) / (float)STEPS2;

    // get rotation quaternation
    glm::vec3 eulers(0.0, 0.0, th);
    glm::quat q = glm::quat(eulers);

    // rotate donut world segment
    for (uint j = 0; j < STEPS1; j++)
    {
      segment1[j] = segment[j] * q;
    }

    // create donut world stripe
    uv[1][0] = (float)i / (float)STEPS2; // UV p1.x
    uv[2][0] = (float)i / (float)STEPS2; // UV p2.x
    uv[0][1] = 0.0f; // UV p0.y
    uv[1][1] = 0.0f; // UV p0.y

    for (uint j = 0; j < STEPS1 - 1; j++)
    {
      uv[3][1] = (float)(j + 1) / (float)(STEPS1 - 1); // UV p3.y
      uv[2][1] = (float)(j + 1) / (float)(STEPS1 - 1); // UV p2.y

      addQuad(vertices,
              segment0[j + 0], segment1[j + 0],
              segment1[j + 1], segment0[j + 1],
              uv[0], uv[1], uv[2], uv[3]
             );

      // shift UV coodinates on y axis
      uv[0][1] = uv[3][1]; // UV p3.y -> p0.y
      uv[1][1] = uv[2][1]; // UV p2.y -> p1.y
    }

    memcpy(segment0, segment1, sizeof(segment0));

    // shift UV coodinates on x axis
    uv[0][0] = uv[1][0]; // UV p1.x -> p0.x
    uv[3][0] = uv[2][0]; // UV p2.x -> p3.x
  }

  // create final donut world stripe (connect with first segment coordinates)
  uv[1][0] = 1.0f; // UV p1.x
  uv[2][0] = 1.0f; // UV p2.x
  uv[0][1] = 0.0f; // UV p0.y
  uv[1][1] = 0.0f; // UV p0.y

  for (uint j = 0; j < STEPS1 - 1; j++)
  {
    uv[3][1] = (float)(j + 1) / (float)(STEPS1 - 1); // UV p3.y
    uv[2][1] = (float)(j + 1) / (float)(STEPS1 - 1); // UV p2.y

    addQuad(vertices,
            segment0[j + 0], segment [j + 0],
            segment [j + 1], segment0[j + 1],
            uv[0], uv[1], uv[2], uv[3]
           );

    // shift UV coodinates on y axis
    uv[0][1] = uv[3][1]; // UV p3.y -> p0.y
    uv[1][1] = uv[2][1]; // UV p2.y -> p1.y
  }

  //fprintf(stderr,"%s, %d: verticesCount=%lu\n",__FILE__,__LINE__,vertices.size());
}

// ---------------------------------------------------------------------

/** generate new random map
 */
LOCAL void generateNewRandomMap()
{
  #if (TEXTURE_TYPE == TEXTURE_TYPE_GENERATED)
    MapGenerator::generate(map, 600, 800);
    for (uint y = 0; y < TEXTURE_HEIGHT; y++)
    {
      for (uint x = 0; x < TEXTURE_WIDTH; x++)
      {
        Tile tile = map.getTile(x,y);

        switch (tile.getType())
        {
          case Tile::Types::WATER:
            textureData[y][x] = Color::interpolate(Color::WATER1, Color::WATER2, (double)rand() / RAND_MAX);
            break;
          case Tile::Types::LAND:
            textureData[y][x] = Color::LAND1;
            break;
          default:
            break;
        }
      }
    }
  #endif
}

/** callback on realize widgets
 * @param widget widget
 */
LOCAL void onRealize(GtkWidget *widget)
{
  GLint status;
  char  s[100];

  gtk_gl_area_make_current(GTK_GL_AREA(widget));

  if (gtk_gl_area_get_error(GTK_GL_AREA(widget)) != NULL)
  {
    return;
  }

  GdkGLContext *context = gtk_gl_area_get_context(GTK_GL_AREA(widget));
  (void)context;

  #if 1
  createDonutWorld(vertices);
  #else
  createCube(vertices);
  #endif

  // init texture
  for (uint y = 0; y < TEXTURE_HEIGHT; y++)
  {
    for (uint x = 0; x < TEXTURE_WIDTH; x++)
    {
      textureData[y][x] = Color::WATER1;
    }
  }

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  #if (TEXTURE_TYPE == TEXTURE_TYPE_GENERATED)
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData.data());
  #else
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, TEXTURE_DATA);
  #endif

  // init vertex buffer
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

  // init shaders
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &VERTEX_SHADER, NULL);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
  glGetShaderInfoLog(vertexShader, sizeof(s), NULL, s);
  //fprintf(stderr,"%s, %d: %s %d\n",__FILE__,__LINE__,s,n);
  assert(status == GL_TRUE);

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &FRAGMENT_SHADER, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
  glGetShaderInfoLog(fragmentShader, sizeof(s), NULL, s);
  //fprintf(stderr,"%s, %d: %s %d\n",__FILE__,__LINE__,s,n);
  assert(status == GL_TRUE);

  // init OpenGL program
  program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  glGetProgramInfoLog(program, sizeof(s), NULL, s);
  //fprintf(stderr,"%s, %d: %s %d\n",__FILE__,__LINE__,s,n);
  assert(status == GL_TRUE);

  // get shader variable locations
  projectionViewModelLocation  = glGetUniformLocation(program, "projectionViewModel");
  textureTranslateLocation     = glGetUniformLocation(program, "textureTranslate");
  GLint vPositionLocation      = glGetAttribLocation(program, "vPosition");
  GLint vColorLocation         = glGetAttribLocation(program, "vColor");
  GLint vUVLocation            = glGetAttribLocation(program, "vUV");
//  GLint colorLocation          = glGetAttribLocation(program, "color");
  GLint textureSamplerLocation = glGetUniformLocation(program, "textureSampler");
#if 0
  fprintf(stderr, "%s, %d: projectionViewModelLocation=%d\n", __FILE__, __LINE__, projectionViewModelLocation);
  fprintf(stderr, "%s, %d: textureTranslateLocation=%d\n", __FILE__, __LINE__, textureTranslateLocation);
  fprintf(stderr, "%s, %d: %d\n", __FILE__, __LINE__, vPositionLocation);
  fprintf(stderr, "%s, %d: %d\n", __FILE__, __LINE__, vColorLocation);
  fprintf(stderr, "%s, %d: vUVLocation=%d\n", __FILE__, __LINE__, vUVLocation);
  fprintf(stderr, "%s, %d: colorLocation=%d\n", __FILE__, __LINE__, colorLocation);
  fprintf(stderr, "%s, %d: textureSamplerLocation=%d\n", __FILE__, __LINE__, textureSamplerLocation);
#endif

  // bind vertex array
  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);
  glEnableVertexAttribArray(vPositionLocation);
  glVertexAttribPointer(vPositionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
  glEnableVertexAttribArray(vColorLocation);
  glVertexAttribPointer(vColorLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));
  glEnableVertexAttribArray(vUVLocation);
  glVertexAttribPointer(vUVLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));

  // bind texture
  glActiveTexture(GL_TEXTURE0);
  //  glBindTexture(GL_TEXTURE_2D, texture);
  glUniform1i(textureSamplerLocation, 0);

  glDetachShader(program, fragmentShader);
  glDetachShader(program, vertexShader);

  // init model projection
  model = glm::rotate(glm::mat4(1.0), (float)(2 * M_PI / 8), glm::vec3(-1, 0, 0));
}

/** callback on unrealize widgets
 * @param widget widget
 */
LOCAL void onUnrealize(GtkWidget *widget)
{
  gtk_gl_area_make_current(GTK_GL_AREA(widget));

  if (gtk_gl_area_get_error(GTK_GL_AREA(widget)) != NULL)
  {
    return;
  }

  glDeleteProgram(program);
  glDeleteShader(fragmentShader);
  glDeleteShader(vertexShader);
  //  glDeleteVertexArray(1, &vertexArray);
  glDeleteBuffers(1, &vertexBuffer);
}

/** redraw scene
 * @param time time
 * @param deltaTime deltaTime of last draw
 */
LOCAL void drawScene(ulong time, ulong deltaTime)
{
  glUseProgram(program);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);

  model = glm::rotate(model, (float)deltaTime / 1000.0f, glm::vec3(0, 0, 1));
  glm::vec3 position            = glm::vec3(0, 0, 2);
  glm::vec3 front               = glm::vec3(0, 0, -1);
  glm::vec3 up                  = glm::vec3(0, 1, 0);
  glm::mat4 view                = glm::lookAt(position, position + front, up);
  glm::mat4 projection          = glm::perspective(45.0, double(VIEW_WIDTH) / double(VIEW_HEIGHT), 0.1, 100.0);
  glm::mat4 projectionViewModel = projection * view * model;
  glUniformMatrix4fv(projectionViewModelLocation, 1, GL_FALSE, (const GLfloat *)&projectionViewModel);

  if (newMapFlag)
  {
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,  // level
                    0,  // x-offset
                    0,  // y-offset
                    TEXTURE_WIDTH,
                    TEXTURE_HEIGHT,
                    GL_RGB,
                    GL_UNSIGNED_BYTE,
                    textureData.data()
                   );
  }

  textureTranslate[1] = (float)((time / 30) % TEXTURE_HEIGHT) / (float)TEXTURE_HEIGHT;
  glUniform2fv(textureTranslateLocation, 1, (const GLfloat *)&textureTranslate);

  glBindVertexArray(vertexArray);

  // draw
  glDrawArrays(GL_TRIANGLES, 0, vertices.size());

  // We finished using the buffers and program
  //  glBindVertexArray(0);
  //  glDisableVertexAttribArray(0);
  //  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
}

/** callback on OpenGL render
 * @param area OpenGL render area
 * @param context OpenGL context
 */
LOCAL gboolean onRender(GtkGLArea    *area,
                        GdkGLContext *context
                       )
{
  if (gtk_gl_area_get_error(area) != NULL)
  {
    return FALSE;
  }

  // get frame delta time
  GDateTime *dateTime = g_date_time_new_now_local();
  ulong time = g_date_time_difference(dateTime, startDateTime) / 1000;
  ulong deltaTime = g_date_time_difference(dateTime, lastDateTime) / 1000;
  lastDateTime = dateTime;

  // clear viewport
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // draw scene
  drawScene(time, deltaTime);

  // flush the contents of the pipeline
  glFlush();
  gtk_gl_area_queue_render(area);

  return TRUE;
}

/** callback on new map
 * @param widget widget
 * @param eventButton event button
 * @param userData user data
 */
LOCAL void onNewMap(GtkWidget *widget, GdkEventButton *eventButton, gpointer userData)
{
  assert(widget != nullptr);

  (void)widget;
  (void)eventButton;
  (void)userData;

  gtk_widget_set_sensitive(GTK_WIDGET(buttonNewMap), FALSE);
  gtk_statusbar_push(GTK_STATUSBAR(statusBar), 0, "Generate new map...");

  auto doneHandler = [](GObject      *sourceObject,
                        GAsyncResult *result,
                        gpointer     userData
                       )
  {
    (void)sourceObject;
    (void)result;
    (void)userData;

    newMapFlag = TRUE;

    gtk_statusbar_pop(GTK_STATUSBAR(statusBar), 0);
    gtk_widget_set_sensitive(buttonNewMap, TRUE);
  };
  GTask *task = g_task_new(widget,nullptr,doneHandler,nullptr);
  assert(task != nullptr);

  auto runHandler = [](GTask        *task,
                       gpointer     sourceObject,
                       gpointer     taskData,
                       GCancellable *cancellable
                      )
  {
    generateNewRandomMap();
  };
  g_task_run_in_thread(task,runHandler);

  g_object_unref(task);
}

/** callback on find islands
 * @param widget widget
 * @param eventButton event button
 * @param userData user data
 */
LOCAL void onFindIslands(GtkWidget *widget, GdkEventButton *eventButton, gpointer userData)
{
  assert(widget != nullptr);

  (void)widget;
  (void)eventButton;
  (void)userData;

  gtk_widget_set_sensitive(GTK_WIDGET(buttonFindIslands), FALSE);
  gtk_statusbar_push(GTK_STATUSBAR(statusBar), 0, "Calculate islands...");

  auto doneHandler = [](GObject      *sourceObject,
                        GAsyncResult *result,
                        gpointer     userData
                       )
  {
    (void)sourceObject;
    (void)userData;

    uint islandCount = static_cast<uint>(g_task_propagate_int(G_TASK(result), nullptr));

    std::stringstream buffer;
    buffer << islandCount;
    gtk_label_set_text(GTK_LABEL(islandsText),buffer.str().c_str());

    gtk_statusbar_pop(GTK_STATUSBAR(statusBar), 0);
    gtk_widget_set_sensitive(buttonFindIslands, TRUE);
  };
  GTask *task = g_task_new(widget,nullptr,doneHandler,nullptr);
  assert(task != nullptr);
  g_task_set_task_data(task, nullptr, nullptr);

  auto runHandler = [](GTask        *task,
                       gpointer     sourceObject,
                       gpointer     taskData,
                       GCancellable *cancellable
                      )
  {
    (void)sourceObject;
    (void)taskData;
    (void)cancellable;

    g_task_return_int(task, map.findIslands());
  };
  g_task_run_in_thread(task,runHandler);

  g_object_unref(task);
}

// ---------------------------------------------------------------------

int main(int argc, char **argv)
{
  GtkWidget *window;

  // initialize GTK
  gtk_init(&argc, &argv);

  // create top level window
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  assert(window != nullptr);
  //  gtk_window_set_default_size(GTK_WINDOW(window), VIEW_WIDTH, VIEW_HEIGHT);
  gtk_window_set_title(GTK_WINDOW(window), "Donut World");
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);

  {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, FALSE);
    assert(vbox != nullptr);

    // view + buttons
    {
      GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, FALSE);
      assert(hbox != nullptr);
      g_object_set(hbox, "margin", 6, NULL);
      gtk_box_set_spacing(GTK_BOX(hbox), 6);
      {
        // view
        {
          GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, FALSE);
          assert(vbox != nullptr);
          gtk_box_set_spacing(GTK_BOX(vbox), 6);
          {
            area = gtk_gl_area_new();
            gtk_widget_set_size_request(GTK_WIDGET(area), VIEW_WIDTH, VIEW_HEIGHT);
            gtk_box_pack_start(GTK_BOX(vbox), area, 1, 1, 0);
            g_signal_connect(area, "realize", G_CALLBACK(onRealize), nullptr);
            g_signal_connect(area, "unrealize", G_CALLBACK(onUnrealize), nullptr);
            g_signal_connect(area, "render", G_CALLBACK(onRender), nullptr);
          }
          gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
        }

        // buttons right
        {
          GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, FALSE);
          assert(vbox != nullptr);
          gtk_box_set_spacing(GTK_BOX(vbox), 6);
          {
            buttonNewMap = gtk_button_new_with_label("New map");
            gtk_box_pack_start(GTK_BOX(vbox), buttonNewMap, FALSE, FALSE, 0);
            g_signal_connect(buttonNewMap, "button-press-event", G_CALLBACK(onNewMap), nullptr);

            buttonFindIslands = gtk_button_new_with_label("Calculate islands");
            gtk_box_pack_start(GTK_BOX(vbox), buttonFindIslands, FALSE, FALSE, 0);
            g_signal_connect(buttonFindIslands, "button-press-event", G_CALLBACK(onFindIslands), nullptr);

            GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, FALSE);
            assert(hbox != nullptr);
            gtk_box_set_spacing(GTK_BOX(hbox), 6);
            {
              GtkWidget *label = gtk_label_new("Islands:");
              gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

              islandsText = gtk_label_new("");
              gtk_box_pack_start(GTK_BOX(hbox), islandsText, FALSE, FALSE, 0);
            }
            gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
          }
          gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
        }
      }
      gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 0);
    }

    // status line
    {
      statusBar = gtk_statusbar_new();
      assert(statusBar != nullptr);
      gtk_statusbar_push(GTK_STATUSBAR(statusBar), 0, "OK");

      gtk_box_pack_start(GTK_BOX(vbox), statusBar, TRUE, TRUE, 0);
    }

    gtk_container_add(GTK_CONTAINER(window), vbox);
  }

  // connect signals
  g_signal_connect(G_OBJECT(window),
                   "delete-event",
                   G_CALLBACK(gtk_main_quit),
                   NULL
                  );

  // start create initial map
  onNewMap(buttonNewMap, nullptr, nullptr);

  // run
  gtk_widget_show_all(GTK_WIDGET(window));
  gtk_main();

  return 0;
}
