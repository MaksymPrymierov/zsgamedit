#include "headers/ProjectEdit.h"
#include "headers/InspectorWin.h"
#include "ui_editor.h"
#include "stdio.h"
#include <iostream>
#include <fstream>
#include <QDir>
#include <QDropEvent>
#include <QFileDialog>

static EditWindow* _editor_win;
static InspectorWin* _inspector_win;

EditWindow::EditWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditWindow)
{
    ui->setupUi(this);

    QObject::connect(ui->fileList, SIGNAL(itemClicked(QListWidgetItem *)),
                this, SLOT(onFileListItemClicked())); //Signal comes, when user clicks on file
    QObject::connect(ui->actionNew_Object, SIGNAL(triggered()),
                this, SLOT(onAddNewGameObject())); //Signal comes, when user clicks on Object->Create
    QObject::connect(ui->actionSave, SIGNAL(triggered()),
                this, SLOT(onSceneSave())); //Signal comes, when user clicks on File->Save
    QObject::connect(ui->actionSave_As, SIGNAL(triggered()),
                this, SLOT(onSceneSaveAs())); //Signal comes, when user clicks on File->Save As

    QObject::connect(ui->objsList, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
                this, SLOT(onObjectListItemClicked())); //Signal comes, when user clicks on File->Save As

    ready = false; //Firstly set it to 0
    hasSceneFile = false;


    setupObjectsHieList();

    ui->objsList->setAcceptDrops(true);
    ui->objsList->setDragEnabled(true);
    ui->objsList->setDragDropMode(QAbstractItemView::InternalMove);
    ui->objsList->world_ptr = &world;

    world.proj_ptr = static_cast<void*>(&project); //Assigning project pointer into world's variable

    ui->fileList->setViewMode(QListView::IconMode);
}

EditWindow::~EditWindow()
{
    delete ui;
}

void EditWindow::init(){

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        //return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);

    this->window = SDL_CreateWindow("Game View", this->width(), 0, 640, 480, SDL_WINDOW_OPENGL); //Create window
    this->glcontext = SDL_GL_CreateContext(window);

    glViewport(0, 0, 640, 480);
    glClearColor(1,0,1,1);

    render = new RenderPipeline;
    render->InitGLEW();

    render->setup();
    ready = true;//Everything is ready

}

void EditWindow::assignIconFile(QListWidgetItem* item){
    if(item->text().endsWith(".txt") || item->text().endsWith(".inf") || item->text().endsWith(".scn")){
        item->setIcon(QIcon::fromTheme("text-x-generic"));
    }
    if(item->text().endsWith(".dds") || item->text().endsWith(".DDS")){
        item->setIcon(QIcon::fromTheme("image-x-generic"));
    }
}

void EditWindow::setViewDirectory(QString dir_path){

    this->current_dir = dir_path;
    this->updateFileList();
}
//Slots
void EditWindow::openFile(QString file_path){

    if(file_path.endsWith(".scn")){ //If it is scene
        setupObjectsHieList(); //Clear everything, at first
        world.openFromFile(file_path, this->column_item_go, ui->objsList); //Open this scene

        scene_path = file_path; //Assign scene path
        hasSceneFile = true; //Scene is saved
    }
}

void EditWindow::onSceneSaveAs(){
    QString filename = QFileDialog::getSaveFileName(this, tr("Save scene file"), project.root_path, "*.scn");
    if(!filename.endsWith(".scn"))
        filename.append(".scn");
    world.saveToFile(filename); //Save to picked file
    scene_path = filename; //Assign scene path
    hasSceneFile = true; //Scene is saved

    updateFileList(); //Make new scene visible in file list
}

void EditWindow::onSceneSave(){
    if(hasSceneFile == false){ //If new created scene without file
        onSceneSaveAs(); //Show dialog and save
    }else{
        world.saveToFile(this->scene_path);
    }
}

void EditWindow::onAddNewGameObject(){
    GameObject* obj_ptr = this->world.newObject(); //Add new object to world
    //this->column_item_go->addChild(obj_ptr->item_ptr); //Add object to list hierarchy
    ui->objsList->addTopLevelItem(obj_ptr->item_ptr);
}

void EditWindow::setupObjectsHieList(){
    QTreeWidget* w_ptr = ui->objsList; //Getting pointer to objects list widget
    w_ptr->clear(); //Clears widget

    column_item_go = new QTreeWidgetItem; //Defining Objects list
    column_item_go->setText(0, "Objects"); //Setting text to Objects

    //w_ptr->addTopLevelItem(column_item_go);
}

void EditWindow::updateFileList(){
    ui->fileList->clear(); //Clear widget content

    QDir directory (this->current_dir);

    directory.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);
    directory.setSorting(QDir::Name | QDir::Reversed);

    QFileInfoList list = directory.entryInfoList(); //Get folder content iterator
    if(this->current_dir.compare(project.root_path)){
        QListWidgetItem* backBtn_item = new QListWidgetItem("(back)", ui->fileList);
        backBtn_item->setIcon(QIcon::fromTheme("go-up"));
    }

    for(int i = 0; i < list.size(); i ++){ //iterate all files, skip 2 last . and ..
        QFileInfo fileInfo = list.at(i);  //get iterated file info
        QListWidgetItem* item = new QListWidgetItem(fileInfo.fileName(), ui->fileList);
        if(fileInfo.isDir()){
            item->setIcon(QIcon::fromTheme("folder"));
        }else{
            assignIconFile(item);
        }
    }
}
//Signal
void EditWindow::onFileListItemClicked(){
    QListWidgetItem* selected_file_item = ui->fileList->currentItem();

    if(selected_file_item->text().compare("(back)") == 0){
        QDir cur_folder = QDir(this->current_dir);
        cur_folder.cdUp();
        setViewDirectory(cur_folder.absolutePath());
        return;
    }

    QDir directory (this->current_dir);

    directory.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);
    directory.setSorting(QDir::Name | QDir::Reversed);

    QFileInfoList list = directory.entryInfoList(); //Get folder content iterator

    for(int i = 0; i < list.size(); i ++){ //iterate all files, skip 2 last . and ..
        QFileInfo fileInfo = list.at(i);  //get iterated file info
        if(fileInfo.fileName().compare(selected_file_item->text()) == 0){ //Find pressed item
            if(fileInfo.isDir()){ //If we pressed on directory
                QString new_path = this->current_dir + "/" + fileInfo.fileName(); //Get directory path
                setViewDirectory(new_path); //go to this
                return; // Exit function to prevent crash
            }else{
                QString new_path = this->current_dir + "/" + fileInfo.fileName(); //Get file path
                openFile(new_path); //Do something to open this file
                return;
            } //If it isn't directory, it is a file
        }
    }
}

void EditWindow::onObjectListItemClicked(){
    _inspector_win->clearContentLayout();
    QTreeWidgetItem* selected_item = ui->objsList->currentItem(); //Obtain pointer to clicked obj item
    if(selected_item->text(0).compare("Objects") == 0) //If we clicked on title
        return; //Exit function to avoid crash

    QString obj_name = selected_item->text(0); //Get label of clicked obj

    GameObject* obj_ptr = world.getObjectByLabel(obj_name); //Obtain pointer to selected object by label
    unsigned int props_num = static_cast<unsigned int>(obj_ptr->properties.size());
    for(unsigned int prop_it = 0; prop_it < props_num; prop_it ++){ //iterate over all properties and send them to inspector
        GameObjectProperty* property_ptr = (obj_ptr->properties[prop_it]); //Obtain pointer to object property
        property_ptr->addPropertyInterfaceToInspector(_inspector_win); //Add its interface to inspector
    }
    _inspector_win->makeAddObjComponentBtn();
    _inspector_win->gameobject_ptr = static_cast<void*>(obj_ptr);
}

void EditWindow::glRender(){
    if(ready == true)
        render->render(this->window);
}

void EditWindow::lookForResources(QString path){
    QDir directory (path); //Creating QDir object
    directory.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);
    directory.setSorting(QDir::DirsLast); //I want to recursive call this function after all files

    QFileInfoList list = directory.entryInfoList(); //Get folder content iterator

    for(int i = 0; i < list.size(); i ++){ //iterate all files, skip 2 last . and ..
        QFileInfo fileInfo = list.at(i);  //get iterated file info

        if(fileInfo.isFile() == true){
            QString name = fileInfo.fileName();
            if(name.endsWith(".DDS") || name.endsWith(".dds")){ //If its an texture
                Resource resource;
                resource.file_path = fileInfo.absoluteFilePath(); //Writing full path
                resource.rel_path = resource.file_path; //Preparing to get relative path
                resource.rel_path.remove(0, project.root_path.size()); //Get relative path by removing length of project root from start
                resource.type = RESOURCE_TYPE_TEXTURE; //Type is texture
                loadResource(&resource); //Perform texture loading to OpenGL
                this->project.resources.push_back(resource);
            }
            if(name.endsWith(".FBX") || name.endsWith(".fbx")){ //If its an mesh
                Resource resource;
                resource.file_path = fileInfo.absoluteFilePath();
                resource.rel_path = resource.file_path; //Preparing to get relative path
                resource.rel_path.remove(0, project.root_path.size()); //Get relative path by removing length of project root from start
                resource.type = RESOURCE_TYPE_MESH; //Type of resource is mesh
                loadResource(&resource); //Perform mesh processing & loading to OpenGL
                this->project.resources.push_back(resource);
            }
        }

        if(fileInfo.isDir() == true){ //If it is directory
            QString newdir_str = path + "/"+ fileInfo.fileName();
            lookForResources(newdir_str); //Call this function inside next dir
        }
    }
}

void EditWindow::loadResource(Resource* resource){
    switch(resource->type){
        case RESOURCE_TYPE_TEXTURE:{ //If resource type is texture
            resource->class_ptr = static_cast<void*>(new ZSPIRE::Texture); //Initialize pointer to texture
            ZSPIRE::Texture* texture_ptr = static_cast<ZSPIRE::Texture*>(resource->class_ptr); //Aquire casted pointer
            std::string str = resource->file_path.toStdString();
            texture_ptr->LoadDDSTextureFromFile(str.c_str()); //Perform texture resource loading
            break;
        }
        case RESOURCE_TYPE_MESH:{
            resource->class_ptr = static_cast<void*>(new ZSPIRE::Mesh); //Initialize pointer to mesh
            ZSPIRE::Mesh* mesh_ptr = static_cast<ZSPIRE::Mesh*>(resource->class_ptr); //Aquire casted pointer
            std::string str = resource->file_path.toStdString();
            mesh_ptr->LoadMeshesFromFileASSIMP(str.c_str());
            break;
        }

    }
}

EditWindow* ZSEditor::openProject(QString conf_file_path){
    _editor_win = new EditWindow(); //Creating class object
    _inspector_win = new InspectorWin();

    //Now reading config file
    std::ifstream project_conf_stream;
    project_conf_stream.open(conf_file_path.toStdString(), std::ifstream::in); //Opening file stream for reading

    while(!project_conf_stream.eof()){ //If reaading finished
        std::string prefix;
        project_conf_stream >> prefix; //Reading prefix
        if(prefix.compare("ver") == 0){ //If reched to ver
            int ver = 0;
            project_conf_stream >> ver; //Reading version
            _editor_win->project.version = ver; //Storing version in project struct
        }
    }

    //These stupid funcs calculate project root directory
    int step = 5;
    int len = conf_file_path.size();
    while(conf_file_path[len - step] != '/'){
          _editor_win->project.label.insert(0, conf_file_path[len - step]);
          step += 1;
    }
    QString proj_root_dir = conf_file_path;
    proj_root_dir.resize(proj_root_dir.length() - step);
    _editor_win->project.root_path = proj_root_dir; //Setting root path
    _editor_win->setViewDirectory(proj_root_dir);

    project_conf_stream.close(); //Close file stream

    return openEditor(); //Return pointer to edit window
}
EditWindow* ZSEditor::openEditor(){
    _editor_win->init();

    _editor_win->lookForResources(_editor_win->project.root_path); //Make a vector of all resource files
    _editor_win->show(); //Show editor window

    _inspector_win->show();
    _inspector_win->move(_inspector_win->x_win_start, 0);

    return _editor_win;
}

InspectorWin* EditWindow::getInspector(){
    return _inspector_win;
}

ObjTreeWgt::ObjTreeWgt(QWidget* parent) : QTreeWidget (parent){

}

void ObjTreeWgt::dropEvent(QDropEvent* event){
    //User dropped object item
    QList<QTreeWidgetItem*> kids = this->selectedItems(); //Get list of selected object(it is moving object)

    GameObject* obj_ptr = world_ptr->getObjectByLabel(kids.at(0)->text(0)); //Receiving pointer to moving object

    QTreeWidgetItem* pparent = kids.at(0)->parent(); //parent of moved object
    if(pparent == nullptr){ //If object hadn't any parent

    }else{ //If object already parented
        GameObjectLink link = obj_ptr->getLinkToThisObject();
        GameObject* pparent_go = world_ptr->getObjectByLabel(pparent->text(0));
        pparent_go->removeChildObject(link); //Remove object from previous parent
    }

    QTreeWidget::dropEvent(event);


    QTreeWidgetItem* nparent = obj_ptr->item_ptr->parent(); //new parent
    if(nparent != nullptr){ //If we moved obj to another parent
        GameObject* nparent_go = world_ptr->getObjectByLabel(nparent->text(0));
        nparent_go->addChildObject(obj_ptr->getLinkToThisObject());
    }else{ //We unparented object
        obj_ptr->hasParent = false;
        this->addTopLevelItem(obj_ptr->item_ptr);
    }
}
