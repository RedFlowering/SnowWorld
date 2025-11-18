"""
하모니아 월드 에디터 유틸리티 위젯 자동 생성 스크립트
Harmonia World Editor Utility Widget Auto-Creation Script

이 스크립트는 언리얼 에디터에서 실행되어 자동으로 에디터 유틸리티 위젯을 생성합니다.

사용법 (Usage):
1. 언리얼 에디터를 엽니다
2. 메뉴: Tools > Execute Python Script...
3. 이 파일을 선택합니다

또는 콘솔에서:
py "Plugins/HarmoniaKit/Source/HarmoniaEditor/WorldGenerator/CreateWorldEditorWidget.py"
"""

import unreal

# 에디터 유틸리티 서브시스템 가져오기
editor_util = unreal.EditorUtilitySubsystem()
editor_asset_lib = unreal.EditorAssetLibrary()
widget_bp_lib = unreal.WidgetBlueprintLibrary()

# 설정
WIDGET_NAME = "EUW_HarmoniaWorldGenerator"
WIDGET_PATH = "/HarmoniaKit/Editor/WorldGenerator"
FULL_ASSET_PATH = f"{WIDGET_PATH}/{WIDGET_NAME}"

def create_world_editor_widget():
    """월드 에디터 유틸리티 위젯 생성"""

    print(f"Creating Harmonia World Editor Utility Widget at: {FULL_ASSET_PATH}")

    # 경로가 존재하는지 확인하고 없으면 생성
    if not editor_asset_lib.does_directory_exist(WIDGET_PATH):
        print(f"Creating directory: {WIDGET_PATH}")
        editor_asset_lib.make_directory(WIDGET_PATH)

    # 이미 존재하는지 확인
    if editor_asset_lib.does_asset_exist(FULL_ASSET_PATH):
        print(f"Warning: Asset already exists at {FULL_ASSET_PATH}")
        response = input("Do you want to overwrite? (yes/no): ")
        if response.lower() != "yes":
            print("Cancelled by user")
            return None
        editor_asset_lib.delete_asset(FULL_ASSET_PATH)

    # 에디터 유틸리티 위젯 블루프린트 생성
    # 참고: HarmoniaWorldEditorUtility를 부모 클래스로 사용

    # 먼저 C++ 클래스 로드
    parent_class = unreal.load_class(None, "/Script/HarmoniaEditor.HarmoniaWorldEditorUtility")

    if not parent_class:
        print("Error: Could not load HarmoniaWorldEditorUtility class!")
        print("Make sure the plugin is compiled and loaded.")
        return None

    # 위젯 블루프린트 생성
    factory = unreal.EditorUtilityWidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    widget_bp = asset_tools.create_asset(
        WIDGET_NAME,
        WIDGET_PATH,
        unreal.WidgetBlueprint,
        factory
    )

    if not widget_bp:
        print("Error: Failed to create widget blueprint!")
        return None

    print(f"Successfully created widget blueprint: {FULL_ASSET_PATH}")

    # UI 레이아웃 구성
    setup_widget_ui(widget_bp)

    # 저장
    editor_asset_lib.save_asset(FULL_ASSET_PATH)

    print("Widget created successfully!")
    print(f"You can now open it from: {FULL_ASSET_PATH}")

    # 자동으로 열기
    editor_util.load_registered_editor_utility(FULL_ASSET_PATH)

    return widget_bp


def setup_widget_ui(widget_bp):
    """위젯 UI 레이아웃 설정"""

    print("Setting up widget UI layout...")

    # 위젯 트리 가져오기
    widget_tree = widget_bp.widget_tree

    if not widget_tree:
        print("Warning: Could not access widget tree")
        return

    # 루트 위젯 (Canvas Panel)
    root = widget_tree.root_widget

    if not root:
        # 루트 생성
        root = widget_tree.construct_widget(unreal.CanvasPanel)
        widget_tree.root_widget = root

    # 메인 레이아웃 (Vertical Box)
    main_vbox = widget_tree.construct_widget(unreal.VerticalBox, name="MainLayout")

    # UI 섹션 생성
    create_title_section(widget_tree, main_vbox)
    create_basic_settings_section(widget_tree, main_vbox)
    create_generation_buttons_section(widget_tree, main_vbox)
    create_terrain_edit_section(widget_tree, main_vbox)
    create_visualization_section(widget_tree, main_vbox)
    create_config_section(widget_tree, main_vbox)
    create_status_section(widget_tree, main_vbox)

    # 메인 레이아웃을 루트에 추가
    slot = root.add_child_to_canvas(main_vbox)

    # 레이아웃 설정 (전체 화면)
    anchors = unreal.Anchors()
    anchors.minimum = unreal.Vector2D(0, 0)
    anchors.maximum = unreal.Vector2D(1, 1)
    slot.set_editor_property("layout_data.anchors", anchors)
    slot.set_editor_property("layout_data.offsets", unreal.Margin(20, 20, 20, 20))

    print("UI layout setup complete")


def create_title_section(widget_tree, parent):
    """타이틀 섹션 생성"""
    title_text = widget_tree.construct_widget(unreal.TextBlock, name="TitleText")
    title_text.set_editor_property("text", unreal.Text("하모니아 월드 생성기 (Harmonia World Generator)"))

    # 폰트 크기 설정
    font = unreal.SlateFontInfo()
    font.size = 24
    title_text.set_editor_property("font", font)

    parent.add_child(title_text)


def create_basic_settings_section(widget_tree, parent):
    """기본 설정 섹션"""
    section_title = widget_tree.construct_widget(unreal.TextBlock, name="BasicSettingsTitle")
    section_title.set_editor_property("text", unreal.Text("기본 설정 (Basic Settings)"))
    parent.add_child(section_title)

    # WorldSizeX
    add_labeled_spinbox(widget_tree, parent, "WorldSizeX", "World Size X:", 64, 8192, 512)

    # WorldSizeY
    add_labeled_spinbox(widget_tree, parent, "WorldSizeY", "World Size Y:", 64, 8192, 512)

    # MaxHeight
    add_labeled_spinbox(widget_tree, parent, "MaxHeight", "Max Height:", 100, 100000, 25600)

    # SeaLevel
    add_labeled_slider(widget_tree, parent, "SeaLevel", "Sea Level:", 0.0, 1.0, 0.4)

    # Seed
    add_labeled_spinbox(widget_tree, parent, "Seed", "Seed:", 0, 999999, 12345)


def create_generation_buttons_section(widget_tree, parent):
    """생성 버튼 섹션"""
    section_title = widget_tree.construct_widget(unreal.TextBlock, name="GenerationTitle")
    section_title.set_editor_property("text", unreal.Text("월드 생성 (Generation)"))
    parent.add_child(section_title)

    hbox = widget_tree.construct_widget(unreal.HorizontalBox, name="GenerationButtons")

    add_button(widget_tree, hbox, "BtnGenerateWorld", "Generate World")
    add_button(widget_tree, hbox, "BtnGenerateTerrain", "Terrain Only")
    add_button(widget_tree, hbox, "BtnGenerateObjects", "Objects Only")
    add_button(widget_tree, hbox, "BtnQuickPreview", "Quick Preview")

    parent.add_child(hbox)


def create_terrain_edit_section(widget_tree, parent):
    """지형 편집 섹션"""
    section_title = widget_tree.construct_widget(unreal.TextBlock, name="TerrainEditTitle")
    section_title.set_editor_property("text", unreal.Text("지형 편집 (Terrain Editing)"))
    parent.add_child(section_title)

    hbox = widget_tree.construct_widget(unreal.HorizontalBox, name="TerrainEditButtons")

    add_button(widget_tree, hbox, "BtnRaiseTerrain", "Raise")
    add_button(widget_tree, hbox, "BtnLowerTerrain", "Lower")
    add_button(widget_tree, hbox, "BtnFlattenTerrain", "Flatten")
    add_button(widget_tree, hbox, "BtnSmoothTerrain", "Smooth")
    add_button(widget_tree, hbox, "BtnCreateCrater", "Crater")
    add_button(widget_tree, hbox, "BtnCreateHill", "Hill")

    parent.add_child(hbox)


def create_visualization_section(widget_tree, parent):
    """시각화 섹션"""
    section_title = widget_tree.construct_widget(unreal.TextBlock, name="VisualizationTitle")
    section_title.set_editor_property("text", unreal.Text("시각화 (Visualization)"))
    parent.add_child(section_title)

    hbox = widget_tree.construct_widget(unreal.HorizontalBox, name="VisualizationButtons")

    add_button(widget_tree, hbox, "BtnShowBiomes", "Biomes")
    add_button(widget_tree, hbox, "BtnShowRivers", "Rivers")
    add_button(widget_tree, hbox, "BtnShowStructures", "Structures")
    add_button(widget_tree, hbox, "BtnShowCaves", "Caves")
    add_button(widget_tree, hbox, "BtnClearAll", "Clear All")

    parent.add_child(hbox)


def create_config_section(widget_tree, parent):
    """설정 관리 섹션"""
    section_title = widget_tree.construct_widget(unreal.TextBlock, name="ConfigTitle")
    section_title.set_editor_property("text", unreal.Text("설정 (Configuration)"))
    parent.add_child(section_title)

    hbox = widget_tree.construct_widget(unreal.HorizontalBox, name="ConfigButtons")

    add_button(widget_tree, hbox, "BtnSaveConfig", "Save Config")
    add_button(widget_tree, hbox, "BtnLoadConfig", "Load Config")
    add_button(widget_tree, hbox, "BtnResetDefaults", "Reset to Defaults")

    parent.add_child(hbox)

    # 프리셋 버튼
    preset_hbox = widget_tree.construct_widget(unreal.HorizontalBox, name="PresetButtons")

    add_button(widget_tree, preset_hbox, "BtnPresetFlat", "Preset: Flat")
    add_button(widget_tree, preset_hbox, "BtnPresetMountains", "Preset: Mountains")
    add_button(widget_tree, preset_hbox, "BtnPresetIslands", "Preset: Islands")
    add_button(widget_tree, preset_hbox, "BtnPresetDesert", "Preset: Desert")

    parent.add_child(preset_hbox)


def create_status_section(widget_tree, parent):
    """상태 표시 섹션"""
    section_title = widget_tree.construct_widget(unreal.TextBlock, name="StatusTitle")
    section_title.set_editor_property("text", unreal.Text("상태 (Status)"))
    parent.add_child(section_title)

    # Progress Bar
    progress_bar = widget_tree.construct_widget(unreal.ProgressBar, name="ProgressBar")
    progress_bar.set_editor_property("percent", 0.0)
    parent.add_child(progress_bar)

    # Status Text
    status_text = widget_tree.construct_widget(unreal.TextBlock, name="StatusText")
    status_text.set_editor_property("text", unreal.Text("Ready"))
    parent.add_child(status_text)


def add_button(widget_tree, parent, name, text):
    """버튼 추가 헬퍼"""
    button = widget_tree.construct_widget(unreal.Button, name=name)

    # 버튼 텍스트
    text_block = widget_tree.construct_widget(unreal.TextBlock, name=f"{name}_Text")
    text_block.set_editor_property("text", unreal.Text(text))

    button.add_child(text_block)
    parent.add_child(button)

    return button


def add_labeled_spinbox(widget_tree, parent, name, label, min_val, max_val, default_val):
    """라벨이 있는 스핀박스 추가"""
    hbox = widget_tree.construct_widget(unreal.HorizontalBox, name=f"{name}_Container")

    # 라벨
    label_text = widget_tree.construct_widget(unreal.TextBlock, name=f"{name}_Label")
    label_text.set_editor_property("text", unreal.Text(label))
    hbox.add_child(label_text)

    # 스핀박스
    spinbox = widget_tree.construct_widget(unreal.SpinBox, name=name)
    spinbox.set_editor_property("min_value", min_val)
    spinbox.set_editor_property("max_value", max_val)
    spinbox.set_editor_property("value", default_val)
    hbox.add_child(spinbox)

    parent.add_child(hbox)
    return spinbox


def add_labeled_slider(widget_tree, parent, name, label, min_val, max_val, default_val):
    """라벨이 있는 슬라이더 추가"""
    hbox = widget_tree.construct_widget(unreal.HorizontalBox, name=f"{name}_Container")

    # 라벨
    label_text = widget_tree.construct_widget(unreal.TextBlock, name=f"{name}_Label")
    label_text.set_editor_property("text", unreal.Text(label))
    hbox.add_child(label_text)

    # 슬라이더
    slider = widget_tree.construct_widget(unreal.Slider, name=name)
    slider.set_editor_property("min_value", min_val)
    slider.set_editor_property("max_value", max_val)
    slider.set_editor_property("value", default_val)
    hbox.add_child(slider)

    # 값 표시
    value_text = widget_tree.construct_widget(unreal.TextBlock, name=f"{name}_Value")
    value_text.set_editor_property("text", unreal.Text(str(default_val)))
    hbox.add_child(value_text)

    parent.add_child(hbox)
    return slider


# 메인 실행
if __name__ == "__main__":
    print("=" * 60)
    print("Harmonia World Editor Utility Widget Creation Script")
    print("=" * 60)

    widget = create_world_editor_widget()

    if widget:
        print("\n✓ Success!")
        print(f"Widget created at: {FULL_ASSET_PATH}")
        print("\nNext steps:")
        print("1. The widget should now be open in the editor")
        print("2. Switch to the 'Graph' tab to bind button events")
        print("3. Connect each button's OnClicked event to the appropriate C++ function")
        print("4. Save the widget blueprint")
        print("\nFor detailed instructions, see:")
        print("Plugins/HarmoniaKit/Source/HarmoniaEditor/WorldGenerator/README_WorldEditorUtility.md")
    else:
        print("\n✗ Failed to create widget")
        print("Please check the error messages above")

    print("=" * 60)
