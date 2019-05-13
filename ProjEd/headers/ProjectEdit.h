#ifndef project_edit_h
#define project_edit_h


#ifndef INCLUDE_PRJ_ONLY

#include <QMainWindow>
#include <QTreeWidget>
#include <QListWidget>
#include <QMenu>
#include <SDL2/SDL.h>
#include <fstream>

#include "EdActions.h"
#include "../../Render/headers/zs-pipeline.h"
#include "../../World/headers/zs-camera.h"
#include "../../Misc/headers/oal_manager.h"

struct Resource;
struct Project;

#include "../../World/headers/World.h"
#include "../../Misc/headers/ProjBuilder.h"
#include "../../Misc/headers/zs_types.h"

enum GO_TRANSFORM_MODE{
    GO_TRANSFORM_MODE_NONE,
    GO_TRANSFORM_MODE_TRANSLATE,
    GO_TRANSFORM_MODE_SCALE,
    GO_TRANSFORM_MODE_ROTATE
};

#define EW_CLOSE_REASON_UNCLOSED 0
#define EW_CLOSE_REASON_PROJLIST 1
#define EW_CLOSE_REASON_BUILD 2

#endif



#include <vector>
#include <QString>

#ifndef INCLUDE_PRJ_ONLY
namespace Ui {
class EditWindow;
}
#endif


#ifndef INCLUDE_PRJ_ONLY
struct EditorInputState{
    bool isLeftBtnHold; //Left mouse button
    bool isRightBtnHold; //Right mouse button
    bool isMidBtnHold; //Middle button (wheel) hold
    bool isLCtrlHold;
    bool isRCtrlHold;
    bool isLAltHold;

    int mouseX;
    int mouseY;
};

struct ObjectTransformState{
    bool isTransforming;
    GameObject* obj_ptr;
    TransformProperty* tprop_ptr; //Pointer to transform property of obj_ptr
    GO_TRANSFORM_MODE transformMode;

    void setTransformOnObject(GO_TRANSFORM_MODE transformMode);

    ObjectTransformState(){ //Default construct
        isTransforming = false;
        obj_ptr = nullptr;
        tprop_ptr = nullptr;
        transformMode = GO_TRANSFORM_MODE_NONE;
    }
};

struct PropertyPaintState{
    bool enabled;
    GameObjectProperty* prop_ptr;

    float time;
    int last_obj = 0;

    PropertyPaintState(){
        time = 0;
        last_obj = -1;
        enabled = false;
        prop_ptr = nullptr;
    }
};

struct EditorSettings{
    int gameViewWin_Width;
    int gameViewWin_Height;

    EditorSettings(){
        gameViewWin_Width = 640;
        gameViewWin_Height = 480;
    }
};

class ObjectCtxMenu;
class FileCtxMenu;
class FileBrowserCtxMenu;

class EditWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void onFileListItemClicked();
    void onObjectListItemClicked();
    void onObjectCtxMenuShow(QPoint point);
    void onFileCtxMenuShow(QPoint point);
    void onCameraToObjTeleport();

    GameObject* onAddNewGameObject();
    void addNewCube();
    void addNewLight();
    void addNewTile();

    void onSceneSave();
    void onSceneSaveAs();
    void onOpenScene();

    void onNewScene();
    void onNewScript();
    void onNewMaterial();

    void onCloseProject();
    void onBuildProject();
    void onRunProject();

    void onUndoPressed();
    void onRedoPressed();

    void toggleCameras();

    void openRenderSettings();
private:
    QString current_dir; //current directory path string
    QString scene_path;
    bool hasSceneFile; //Is scene saved or loaded

    SDL_Window* window; //Editor preview sdl2 window
    SDL_GLContext glcontext; //Editor preview window opengl context

    RenderPipeline* render;

    ObjectCtxMenu* obj_ctx_menu; //Context menu on object right click
    FileCtxMenu* file_ctx_menu;

    float cam_pitch = 0;
    float cam_yaw = 0;

    QString sheduled_world;
    bool hasSheduledWorld;
public:
    float deltaTime;
    bool ready; //Is everything loaded?
    int close_reason;
    bool isSceneRun; //Is scene running right now
    bool isSceneCamera; //Use World's camera to render world?

    World world;
    ZSPIRE::Camera edit_camera; //Camera to show editing scene
    Project project; //info about current project
    EditorInputState input_state;
    ObjectTransformState obj_trstate; //Describes object transform
    PropertyPaintState ppaint_state; //Describes property painting feature state
    WorldSnapshot run_world_snapshot; //useful to recover world state after running

    EditorSettings settings; //seetings for editor

    QString getCurrentDirectory();

    void init();
    void updateFileList(); //Updates content in file list widget
    void setupObjectsHieList();
    void setViewDirectory(QString dir_path);

    void lookForResources(QString path); //Recursively walk through game directory and load founded resources
    void loadResource(Resource* resource);

    QString createNewTextFile(QString directory, QString name, QString ext, std::string content);
    void openFile(QString file_path); //Useful to open a file
    void addFileToObjectList(QString file_path);
    void assignIconFile(QListWidgetItem* item);

    void runWorld();
    void stopWorld();
    void sheduleWorldLoad(QString file_path);

    void glRender(); //Invoke opengl rendering

    RenderPipeline* getRenderPipeline();
    InspectorWin* getInspector();
    //SDL2 input events stuff
    void onLeftBtnClicked(int X, int Y);
    void onRightBtnClicked(int X, int Y);
    void onMouseMotion(int relX, int relY);
    void onMouseWheel(int x, int y);
    void onKeyDown(SDL_Keysym sym);

    Ui::EditWindow *ui;
    explicit EditWindow(QWidget *parent = nullptr);
    ~EditWindow();

    void callObjectDeletion(GameObjectLink link);
private:

};

class ObjectCtxMenu : public QObject{
    Q_OBJECT
public slots:
    void onDeleteClicked();
    void onDublicateClicked();

    void onMoveClicked();
    void onScaleClicked();
    void onRotateClicked();

    void onStorePrefabPressed();
    void onInfoPressed();
private:
    GameObject* obj_ptr;
    QMenu* menu; //Menu object to contain everything

    QAction* action_dub; //Button to dublicate object
    QAction* action_delete; //Button to delete object

    QAction* action_move;
    QAction* action_scale;
    QAction* action_rotate;

    QAction* store_to_prefab;

    QAction* object_info; //Show dialog with detailed object information
public:
    bool displayTransforms;
    EditWindow* win_ptr;

    ObjectCtxMenu(EditWindow* win, QWidget* parent = nullptr);
    void setObjectPtr(GameObject* obj_ptr);
    void show(QPoint point);
    void close();
};

class FileCtxMenu : public QObject{
    Q_OBJECT
public slots:
    void onDeleteClicked();
    void onRename();

    void onOpenInExplorerPressed();
private:
    QMenu* menu; //Menu object to contain everything

    QAction* action_rename; //Button to dublicate object
    QAction* action_delete; //Button to delete object

    QAction* action_open_in_explorer; //Button to delete object
public:
    EditWindow* win_ptr;
    QString file_path; //path to selected file
    QString file_name;
    QString directory;

    FileCtxMenu(EditWindow* win, QWidget* parent = nullptr);
    void show(QPoint point);
    void close();
};

class FileDeleteDialog : public QDialog{
    Q_OBJECT
private:
    QPushButton del_btn;
    QPushButton close_btn;

    QGridLayout contentLayout;
    QLabel del_message;
public slots:
    void onDelButtonPressed();

public:
    QString file_path;

    FileDeleteDialog(QString file_path, QWidget* parent = nullptr);
};

class FileRenameDialog : public QDialog{
    Q_OBJECT
private:
    QPushButton del_btn;
    QPushButton close_btn;

    QGridLayout contentLayout;
    QLabel rename_message;
    QLineEdit edit_field;
public slots:
    void onRenameButtonPressed();

public:
    QString file_path;
    QString file_name;

    FileRenameDialog(QString file_path, QString file_name, QWidget* parent = nullptr);
};


//Class to represent tree widget
class ObjTreeWgt : public QTreeWidget{
    Q_OBJECT
protected:
    void mousePressEvent(QMouseEvent *event) override;
signals:
    void onRightClick(QPoint pos);
    void onLeftClick(QPoint pos);
public:
    EditWindow* win_ptr;
    ObjTreeWgt(QWidget* parent = nullptr);

    void dropEvent(QDropEvent* event);

    World* world_ptr;
};

class FileListWgt : public QListWidget{
    Q_OBJECT
protected:
    void mousePressEvent(QMouseEvent *event) override;
signals:
    void onRightClick(QPoint pos);
    void onLeftClick(QPoint pos);
public:
    EditWindow* win_ptr;
    FileListWgt(QWidget* parent = nullptr);

    World* world_ptr;
};

namespace ZSEditor {
    EditWindow* openProject(Project& project);
    EditWindow* openEditor();
}

EdActions* getActionManager();
#endif

#endif
