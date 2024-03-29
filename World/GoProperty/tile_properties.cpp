#include "../../ProjEd/headers/ProjectEdit.h"
#include "../headers/World.h"

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef _WIN32
#include <Windows.h> //For Sleep();
#endif

#include "../headers/2dtileproperties.h"
#include "../headers/obj_properties.h"
#include "../../ProjEd/headers/InspEditAreas.h"

extern InspectorWin* _inspector_win;
static TileGroupProperty* current_property; //Property, that shown

void onCreateBtnPress(){
    current_property->process();
}

void onClearBtnPress(){
    current_property->clear();
}

TileGroupProperty::TileGroupProperty(){
    type = GO_PROPERTY_TYPE_TILE_GROUP; //Set correct type
    active = true; //And it is active

    this->isCreated = false;
    this->tiles_amount_X = 0;
    this->tiles_amount_Y = 0;

    this->mesh_string = "@plane";
    this->diffuse_relpath = "@none";
}

TileProperty::TileProperty(){
    type = GO_PROPERTY_TYPE_TILE;
    active = true;
    lastAnimState = false;

    this->texture_diffuse = nullptr;
    this->diffuse_relpath = "@none";

    this->texture_transparent = nullptr;
    this->transparent_relpath = "@none";
}

void TileGroupProperty::addPropertyInterfaceToInspector(InspectorWin* inspector){
    if(!isCreated){ //If tiles haven't been initialized
        IntPropertyArea* tileSizeXArea = new IntPropertyArea;
        tileSizeXArea->setLabel("Tile Width");
        tileSizeXArea->go_property = static_cast<void*>(this);
        tileSizeXArea->value = &this->geometry.tileWidth;
        inspector->addPropertyArea(tileSizeXArea);

        IntPropertyArea* tileSizeYArea = new IntPropertyArea;
        tileSizeYArea->setLabel("Tile Height");
        tileSizeYArea->go_property = static_cast<void*>(this);
        tileSizeYArea->value = &this->geometry.tileHeight;
        inspector->addPropertyArea(tileSizeYArea);

        IntPropertyArea* tilesAmountX = new IntPropertyArea;
        tilesAmountX->setLabel("Tiles amount X");
        tilesAmountX->go_property = static_cast<void*>(this);
        tilesAmountX->value = &this->tiles_amount_X;
        inspector->addPropertyArea(tilesAmountX);

        IntPropertyArea* tilesAmountY = new IntPropertyArea;
        tilesAmountY->setLabel("Tiles amount Y");
        tilesAmountY->go_property = static_cast<void*>(this);
        tilesAmountY->value = &this->tiles_amount_Y;
        inspector->addPropertyArea(tilesAmountY);
        //Resources pick
        PickResourceArea* mesh_area = new PickResourceArea(RESOURCE_TYPE_MESH);
        mesh_area->setLabel("Tiles Mesh");
        mesh_area->go_property = static_cast<void*>(this);
        mesh_area->rel_path = &this->mesh_string;
        mesh_area->isShowNoneItem = false;
        inspector->addPropertyArea(mesh_area);

        PickResourceArea* diffuse_area = new PickResourceArea(RESOURCE_TYPE_TEXTURE);
        diffuse_area->setLabel("Tiles Diffuse");
        diffuse_area->go_property = static_cast<void*>(this);
        diffuse_area->rel_path = &this->diffuse_relpath;
        diffuse_area->isShowNoneItem = true;
        inspector->addPropertyArea(diffuse_area);

        //Add button to add objects
        AreaButton* btn = new AreaButton;
        btn->onPressFuncPtr = &onCreateBtnPress;
        btn->button->setText("Process"); //Setting text to qt button
        inspector->getContentLayout()->addWidget(btn->button);
        btn->insp_ptr = inspector; //Setting inspector pointer
        inspector->registerUiObject(btn);
    }else{
        QString out = "";
        out += ("Tiles X : " + QString::number(this->tiles_amount_X) + QString(" \n"));
        out += "Tiles Y : " + QString::number(this->tiles_amount_Y) + QString(" \n");
        out += ("Tile Width : " + QString::number(this->geometry.tileWidth) + QString(" \n"));
        out += "Tile Height : " + QString::number(this->geometry.tileHeight) + QString(" \n");

        AreaText* group_info = new AreaText;
        group_info->label->setText(out);
        inspector->getContentLayout()->addWidget(group_info->label);
        inspector->registerUiObject(group_info);

        AreaButton* btn = new AreaButton;
        btn->onPressFuncPtr = &onClearBtnPress;
        btn->button->setText("Clear");
        inspector->getContentLayout()->addWidget(btn->button);
        btn->insp_ptr = inspector;
        inspector->registerUiObject(btn);
    }
    current_property = this;
}

void TileGroupProperty::process(){
    //receive pointer to object that own this property
    getActionManager()->newSnapshotAction(go_link.world_ptr);
    World* wrld = world_ptr;

    for(int x_i = 0; x_i < tiles_amount_X; x_i ++){
        for(int y_i = 0; y_i < tiles_amount_Y; y_i ++){
#ifdef __linux__
            usleep(1100); //Wait some time to make random generator work properly
#endif
#ifdef _WIN32
            Sleep(10);
#endif
            GameObject* obj = wrld->newObject(); //Invoke new object creation

            go_link.updLinkPtr();
            GameObject* parent = go_link.ptr;
            TransformProperty* parent_transform = parent->getTransformProperty();
            LabelProperty* parent_label = parent->getLabelProperty();

            //obj->render_type = GO_RENDER_TYPE_TILE;
            obj->addProperty(GO_PROPERTY_TYPE_MESH); //Adding mesh
            obj->addProperty(GO_PROPERTY_TYPE_TILE); //Adding tile
            //Receive properties ptrs
            TransformProperty* transform = obj->getTransformProperty();
            LabelProperty* label = obj->getLabelProperty();
            TileProperty* tile_prop = static_cast<TileProperty*>(obj->getPropertyPtrByType(GO_PROPERTY_TYPE_TILE));
            MeshProperty* mesh_prop = static_cast<MeshProperty*>(obj->getPropertyPtrByType(GO_PROPERTY_TYPE_MESH));

            mesh_prop->resource_relpath = this->mesh_string; //Default plane as mesh
            mesh_prop->updateMeshPtr(); //Update mesh pointer in property
            tile_prop->geometry = this->geometry; //Assign geometry property
            tile_prop->diffuse_relpath = this->diffuse_relpath; //Copy texture relpath
            tile_prop->updTexturePtr(); //Find texture pointer

            transform->scale = ZSVECTOR3(geometry.tileWidth, geometry.tileHeight, 1);
            transform->translation = ZSVECTOR3(geometry.tileWidth * x_i * 2, geometry.tileHeight * y_i * 2, 0);
            transform->translation = transform->translation + parent_transform->translation;

            label->label = parent_label->label + QString::number(x_i) + "," + QString::number(y_i); //Get new object new name
            obj->item_ptr->setText(0, label->label);

            parent->addChildObject(obj->getLinkToThisObject()); //Make new object dependent
            parent->item_ptr->addChild(obj->item_ptr); //Add widget as a child to tree
        }
    }
    this->isCreated = true;
}

void TileGroupProperty::clear(){
    go_link.updLinkPtr();
    //Create snapshot
    getActionManager()->newSnapshotAction(go_link.world_ptr);

    GameObject* parent = go_link.ptr;
    unsigned int children_am = static_cast<unsigned int>(parent->children.size());
    for(unsigned int ch_i = 0; ch_i < children_am; ch_i ++){
        GameObjectLink link_toremove = parent->children[0];
        world_ptr->removeObj(link_toremove);
        go_link.updLinkPtr();
        parent = go_link.ptr;
    }

    isCreated = false;
}

void TileGroupProperty::copyTo(GameObjectProperty* dest){
    if(dest->type != this->type) return; //if it isn't transform

    //Do base things
    GameObjectProperty::copyTo(dest);

    TileGroupProperty* _dest = static_cast<TileGroupProperty*>(dest);
    //Transfer all variables
    _dest->geometry = geometry;
    _dest->tiles_amount_X = tiles_amount_X;
    _dest->tiles_amount_Y = tiles_amount_Y;
    _dest->isCreated = isCreated;
}

void TileProperty::addPropertyInterfaceToInspector(InspectorWin* inspector){
    BoolCheckboxArea* isAnim = new BoolCheckboxArea;
    isAnim->setLabel("Animated ");
    isAnim->go_property = static_cast<void*>(this);
    isAnim->bool_ptr = &this->anim_property.isAnimated;
    inspector->addPropertyArea(isAnim);

    if(this->anim_property.isAnimated){ //if animation turned on
        IntPropertyArea* rowsAmount = new IntPropertyArea;
        rowsAmount->setLabel("Atlas rows");
        rowsAmount->go_property = static_cast<void*>(this);
        rowsAmount->value = &this->anim_property.framesX;
        inspector->addPropertyArea(rowsAmount);

        IntPropertyArea* colsAmount = new IntPropertyArea;
        colsAmount->setLabel("Atlas cols");
        colsAmount->go_property = static_cast<void*>(this);
        colsAmount->value = &this->anim_property.framesY;
        inspector->addPropertyArea(colsAmount);

        IntPropertyArea* frameTime = new IntPropertyArea;
        frameTime->setLabel("frame time");
        frameTime->go_property = static_cast<void*>(this);
        frameTime->value = &this->anim_property.frame_time;
        inspector->addPropertyArea(frameTime);
    }

    PickResourceArea* area = new PickResourceArea(RESOURCE_TYPE_TEXTURE);
    area->setLabel("Texture");
    area->go_property = static_cast<void*>(this);
    area->rel_path = &diffuse_relpath;
    inspector->addPropertyArea(area);

    PickResourceArea* transparent_area = new PickResourceArea(RESOURCE_TYPE_TEXTURE);
    transparent_area->setLabel("Secondary Diffuse");
    transparent_area->go_property = static_cast<void*>(this);
    transparent_area->rel_path = &transparent_relpath;
    inspector->addPropertyArea(transparent_area);
}

void TileProperty::onValueChanged(){
    updTexturePtr();

    if(lastAnimState != this->anim_property.isAnimated){
        _inspector_win->updateRequired = true;
        lastAnimState = this->anim_property.isAnimated;
    }
}

void TileProperty::updTexturePtr(){
    //Update color texture
    this->texture_diffuse = world_ptr->getTexturePtrByRelPath(diffuse_relpath);
    //Update transparent layer texture
    this->texture_transparent = world_ptr->getTexturePtrByRelPath(transparent_relpath);
}

void TileProperty::onAddToObject(){

}

void TileProperty::copyTo(GameObjectProperty* dest){
    if(dest->type != this->type) return; //if it isn't transform

    //Do base things
    GameObjectProperty::copyTo(dest);

    TileProperty* _dest = static_cast<TileProperty*>(dest);
    _dest->diffuse_relpath = diffuse_relpath;
    _dest->texture_diffuse = texture_diffuse;
    _dest->anim_property = anim_property;
    _dest->geometry = geometry;
}
void TileProperty::onUpdate(float deltaTime){
    if(this->anim_state.playing == true){ //if we playing anim
        anim_state.current_time += deltaTime;
        if(anim_state.current_time >= anim_property.frame_time){ //if its time to switch frame
            anim_state.current_time = 0; //make time zero
            anim_state.current_frame += 1; //switch to next frame

            anim_state.cur_frameX = anim_state.current_frame % anim_property.framesX;
            anim_state.cur_frameY = anim_state.current_frame / anim_property.framesX;
        }
        //if we played all the frames
        if(anim_state.current_frame > anim_property.framesX * anim_property.framesY){
            //start from beginning
            anim_state.current_time = 0;
            anim_state.current_frame = 0;

            anim_state.cur_frameX = 0;
            anim_state.cur_frameY = 0;
        }

    }
}

void TileProperty::playAnim(){
    anim_state.playing = true;
}
void TileProperty::stopAnim(){
    anim_state.playing = false;
}
void TileProperty::setDiffuseTexture(std::string texture){
    diffuse_relpath = QString::fromStdString(texture);
        //Update texture pointer
    updTexturePtr();
}
