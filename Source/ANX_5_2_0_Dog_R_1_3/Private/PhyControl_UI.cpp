// PhyControl_UI.cpp
// Physics Control 用户界面实现 - 事件驱动版本

#include "PhyControl_UI.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"

// ==================== 颜色定义 ====================
namespace PhyControlColors
{
    const FLinearColor Background(0.1f, 0.1f, 0.1f, 0.85f);
    const FLinearColor PanelBorder(0.2f, 0.2f, 0.2f, 1.0f);
    const FLinearColor ActiveGreen(0.3f, 0.85f, 0.3f, 1.0f);
    const FLinearColor InactiveGray(0.4f, 0.4f, 0.4f, 1.0f);
    const FLinearColor SliderTrack(0.15f, 0.15f, 0.15f, 1.0f);
    const FLinearColor SliderThumb(0.7f, 0.7f, 0.7f, 1.0f);
    const FLinearColor ButtonOrange(0.9f, 0.6f, 0.1f, 1.0f);
    const FLinearColor TextWhite(1.0f, 1.0f, 1.0f, 1.0f);
}

// ==================== 尺寸定义 ====================
namespace PhyControlSizes
{
    constexpr float PanelWidth = 500.0f;
    constexpr float PanelHeight = 300.0f;
    constexpr float SliderTrackHeight = 120.0f;
    constexpr float SliderTrackWidth = 4.0f;
    constexpr float SliderThumbWidth = 30.0f;   // 横向长方形
    constexpr float SliderThumbHeight = 8.0f;
    constexpr float CircleButtonSize = 28.0f;
    constexpr float BottomButtonWidth = 45.0f;
    constexpr float BottomButtonHeight = 35.0f;
}

// ==================== 公共方法 ====================

void UPhyControl_UI::SetControlData(const FPhyControlUIData& InData)
{
    ControlData = InData;
    RefreshUI();
}

void UPhyControl_UI::SetTargetLimb(FName LimbName)
{
    ControlData.TargetLimb = LimbName;
}

void UPhyControl_UI::RefreshUI()
{
    UpdateIndicator(WorldIndicator, ControlData.WorldSpace.bEnabled);
    UpdateIndicator(ParentIndicator, ControlData.ParentSpace.bEnabled);
    
    UpdateCircleButton(ButtonP_Indicator, ControlData.BodyModifier.bPhysicsEnabled);
    UpdateCircleButton(ButtonG_Indicator, ControlData.BodyModifier.bGravityEnabled);
    UpdateCircleButton(ButtonS_Indicator, ControlData.BodyModifier.bSimulated);
    UpdateCircleButton(ButtonM_Indicator, true);
    UpdateCircleButton(ButtonB_Indicator, ControlData.BodyModifier.BlendWeight > 0.5f);
    UpdateCircleButton(ButtonT_Indicator, ControlData.Options.bUseSkeletalAnimation);
}

void UPhyControl_UI::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    
    RootWidget.Reset();
    WorldIndicator.Reset();
    ParentIndicator.Reset();
    ButtonP_Indicator.Reset();
    ButtonG_Indicator.Reset();
    ButtonS_Indicator.Reset();
    ButtonM_Indicator.Reset();
    ButtonB_Indicator.Reset();
    ButtonT_Indicator.Reset();
    ButtonR.Reset();
    ButtonI.Reset();
    ButtonW.Reset();
    ButtonS_Bottom.Reset();
}

// ==================== UI 更新 ====================

void UPhyControl_UI::UpdateIndicator(TSharedPtr<SImage>& Indicator, bool bEnabled)
{
    if (Indicator.IsValid())
    {
        Indicator->SetImage(GetIndicatorBrush(bEnabled));
    }
}

void UPhyControl_UI::UpdateCircleButton(TSharedPtr<SBorder>& Indicator, bool bEnabled)
{
    if (Indicator.IsValid())
    {
        Indicator->SetBorderBackgroundColor(
            bEnabled ? PhyControlColors::ActiveGreen : PhyControlColors::InactiveGray
        );
    }
}

// ==================== 蓝图可重载默认实现 ====================

FSlateBrush UPhyControl_UI::GetIndicatorBrush_Implementation(bool bEnabled)
{
    // 默认实现：返回纯色 Brush
    FSlateBrush Brush;
    Brush.TintColor = FSlateColor(bEnabled ? 
        PhyControlColors::ActiveGreen : 
        PhyControlColors::InactiveGray);
    return Brush;
}

// ==================== UI 构建 ====================

TSharedRef<SWidget> UPhyControl_UI::RebuildWidget()
{
    using namespace PhyControlSizes;
    
    RootWidget = 
        // 外层：屏幕底部居中
        SNew(SBox)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Bottom)
        .Padding(FMargin(0, 0, 0, 40))
        [
            // 固定尺寸面板
            SNew(SBox)
            .WidthOverride(PanelWidth)
            [
                SNew(SBorder)
                .BorderBackgroundColor(PhyControlColors::Background)
                .Padding(FMargin(20.0f))
                [
                    SNew(SVerticalBox)
                    
                    // ========== Slot[0]: 主控制区 ==========
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        
                        // World 组
                        + SHorizontalBox::Slot()
                        .FillWidth(1.0f)
                        .Padding(0, 0, 15, 0)
                        [
                            CreateControlSpacePanel(
                                TEXT("World"),
                                &ControlData.WorldSpace,
                                WorldIndicator,
                                [this]() { OnWorldToggle(); }
                            )
                        ]
                        
                        // Parent 组
                        + SHorizontalBox::Slot()
                        .FillWidth(1.0f)
                        .Padding(15, 0, 15, 0)
                        [
                            CreateControlSpacePanel(
                                TEXT("Parent"),
                                &ControlData.ParentSpace,
                                ParentIndicator,
                                [this]() { OnParentToggle(); }
                            )
                        ]
                        
                        // 右侧按钮列
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(15, 0, 0, 0)
                        .VAlign(VAlign_Top)
                        [
                            CreateRightButtonColumn()
                        ]
                    ]
                    
                    // ========== Slot[1]: T + 水平滑块 ==========
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0, 20, 0, 0)
                    [
                        SNew(SHorizontalBox)
                        
                        // T 按钮
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        .Padding(0, 0, 15, 0)
                        [
                            CreateCircleButton(TEXT("T"), &ControlData.Options.bUseSkeletalAnimation, ButtonT_Indicator,
                                [this]()
                                {
                                    ControlData.Options.bUseSkeletalAnimation = !ControlData.Options.bUseSkeletalAnimation;
                                    UpdateCircleButton(ButtonT_Indicator, ControlData.Options.bUseSkeletalAnimation);
                                    OnBoolChanged.Broadcast(FName("UseSkeletalAnimation"), ControlData.Options.bUseSkeletalAnimation);
                                    NotifyDataChanged();
                                })
                        ]
                        
                        // 水平滑块
                        + SHorizontalBox::Slot()
                        .FillWidth(1.0f)
                        .VAlign(VAlign_Center)
                        [
                            CreateCustomHorizontalSlider(
                                &ControlData.BodyModifier.Multiplier,
                                0.0f, 2.0f,
                                [this](float V)
                                {
                                    OnValueChanged.Broadcast(FName("GlobalMultiplier"), V);
                                    NotifyDataChanged();
                                }
                            )
                        ]
                    ]
                    
                    // ========== Slot[2]: 底部按钮 ==========
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0, 20, 0, 0)
                    [
                        SNew(SHorizontalBox)
                        
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(0, 0, 8, 0)
                        [
                            CreateBottomButton(TEXT("R"), ButtonR, [this]() { OnResetRequested(); })
                        ]
                        
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(0, 0, 8, 0)
                        [
                            CreateBottomButton(TEXT("I"), ButtonI, [this]() { OnInitializeRequested(); })
                        ]
                        
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(0, 0, 8, 0)
                        [
                            CreateBottomButton(TEXT("W"), ButtonW, [this]() 
                            {
                                ControlData.Options.bWorldSpaceOverride = !ControlData.Options.bWorldSpaceOverride;
                                OnBoolChanged.Broadcast(FName("WorldSpaceOverride"), ControlData.Options.bWorldSpaceOverride);
                                NotifyDataChanged();
                            })
                        ]
                        
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            CreateBottomButton(TEXT("S"), ButtonS_Bottom, [this]() 
                            {
                                ControlData.Options.bUseSkeletalAnimation = !ControlData.Options.bUseSkeletalAnimation;
                                UpdateCircleButton(ButtonT_Indicator, ControlData.Options.bUseSkeletalAnimation);
                                OnBoolChanged.Broadcast(FName("UseSkeletalAnimation"), ControlData.Options.bUseSkeletalAnimation);
                                NotifyDataChanged();
                            })
                        ]
                    ]
                ]
            ]
        ];

    return RootWidget.ToSharedRef();
}

// ==================== 控制空间面板 ====================

TSharedRef<SWidget> UPhyControl_UI::CreateControlSpacePanel(
    const FString& Title,
    FPhyControlSpaceData* SpaceData,
    TSharedPtr<SImage>& OutIndicator,
    TFunction<void()> OnToggled)
{
    return SNew(SVerticalBox)
        
        // 标题行
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 15)
        [
            SNew(SHorizontalBox)
            
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 10, 0)
            [
                CreateIndicatorButton(&SpaceData->bEnabled, OutIndicator, OnToggled)
            ]
            
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Title))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
                .ColorAndOpacity(PhyControlColors::TextWhite)
            ]
        ]
        
        // 滑块区
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .HAlign(HAlign_Center)
            [
                CreateCustomVerticalSlider(TEXT("Strength"), &SpaceData->Strength,
                    [this](float V) { NotifyDataChanged(); })
            ]
            
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .HAlign(HAlign_Center)
            [
                CreateCustomVerticalSlider(TEXT("Damping"), &SpaceData->Damping,
                    [this](float V) { NotifyDataChanged(); })
            ]
        ];
}

// ==================== 右侧按钮列 ====================

TSharedRef<SWidget> UPhyControl_UI::CreateRightButtonColumn()
{
    return SNew(SVerticalBox)
        
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 5)
        [
            CreateCircleButton(TEXT("P"), &ControlData.BodyModifier.bPhysicsEnabled, ButtonP_Indicator,
                [this]()
                {
                    ControlData.BodyModifier.bPhysicsEnabled = !ControlData.BodyModifier.bPhysicsEnabled;
                    UpdateCircleButton(ButtonP_Indicator, ControlData.BodyModifier.bPhysicsEnabled);
                    OnBoolChanged.Broadcast(FName("PhysicsEnabled"), ControlData.BodyModifier.bPhysicsEnabled);
                    NotifyDataChanged();
                })
        ]
        
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 5)
        [
            CreateCircleButton(TEXT("G"), &ControlData.BodyModifier.bGravityEnabled, ButtonG_Indicator,
                [this]()
                {
                    ControlData.BodyModifier.bGravityEnabled = !ControlData.BodyModifier.bGravityEnabled;
                    UpdateCircleButton(ButtonG_Indicator, ControlData.BodyModifier.bGravityEnabled);
                    OnBoolChanged.Broadcast(FName("GravityEnabled"), ControlData.BodyModifier.bGravityEnabled);
                    NotifyDataChanged();
                })
        ]
        
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 5)
        [
            CreateCircleButton(TEXT("S"), &ControlData.BodyModifier.bSimulated, ButtonS_Indicator,
                [this]()
                {
                    ControlData.BodyModifier.bSimulated = !ControlData.BodyModifier.bSimulated;
                    UpdateCircleButton(ButtonS_Indicator, ControlData.BodyModifier.bSimulated);
                    OnBoolChanged.Broadcast(FName("Simulated"), ControlData.BodyModifier.bSimulated);
                    NotifyDataChanged();
                })
        ]
        
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 5)
        [
            CreateCircleButton(TEXT("M"), nullptr, ButtonM_Indicator,
                [this]()
                {
                    ControlData.BodyModifier.Multiplier += 0.5f;
                    if (ControlData.BodyModifier.Multiplier > 2.0f)
                        ControlData.BodyModifier.Multiplier = 0.5f;
                    OnValueChanged.Broadcast(FName("Multiplier"), ControlData.BodyModifier.Multiplier);
                    NotifyDataChanged();
                })
        ]
        
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            CreateCircleButton(TEXT("B"), nullptr, ButtonB_Indicator,
                [this]()
                {
                    ControlData.BodyModifier.BlendWeight += 0.25f;
                    if (ControlData.BodyModifier.BlendWeight > 1.0f)
                        ControlData.BodyModifier.BlendWeight = 0.0f;
                    UpdateCircleButton(ButtonB_Indicator, ControlData.BodyModifier.BlendWeight > 0.5f);
                    OnValueChanged.Broadcast(FName("BlendWeight"), ControlData.BodyModifier.BlendWeight);
                    NotifyDataChanged();
                })
        ];
}

// ==================== 辅助控件 ====================

TSharedRef<SWidget> UPhyControl_UI::CreateIndicatorButton(
    bool* EnabledPtr,
    TSharedPtr<SImage>& OutIndicator,
    TFunction<void()> OnClicked)
{
    bool bInitialEnabled = EnabledPtr && *EnabledPtr;

    return SNew(SButton)
        .ButtonStyle(FAppStyle::Get(), "NoBorder")
        .OnClicked_Lambda([this, EnabledPtr, &OutIndicator, OnClicked]()
        { 
            if (EnabledPtr)
            {
                *EnabledPtr = !(*EnabledPtr);
                UpdateIndicator(OutIndicator, *EnabledPtr);
            }
            if (OnClicked) OnClicked();
            return FReply::Handled(); 
        })
        [
            SAssignNew(OutIndicator, SImage)
            .DesiredSizeOverride(FVector2D(24, 24))
            .Image(GetIndicatorBrush(bInitialEnabled))  // 调用可重载方法
        ];
}

TSharedRef<SWidget> UPhyControl_UI::CreateCircleButton(
    const FString& Label,
    bool* EnabledPtr,
    TSharedPtr<SBorder>& OutIndicator,
    TFunction<void()> OnClicked)
{
    using namespace PhyControlSizes;
    
    bool bInitialEnabled = (EnabledPtr && *EnabledPtr) || (Label == TEXT("M"));
    FLinearColor InitialColor = bInitialEnabled ? 
        PhyControlColors::ActiveGreen : PhyControlColors::InactiveGray;

    return SNew(SButton)
        .ButtonStyle(FAppStyle::Get(), "NoBorder")
        .OnClicked_Lambda([OnClicked]()
        { 
            if (OnClicked) OnClicked();
            return FReply::Handled(); 
        })
        [
            SAssignNew(OutIndicator, SBorder)
            .BorderBackgroundColor(InitialColor)
            .Padding(0)
            [
                SNew(SBox)
                .WidthOverride(CircleButtonSize)
                .HeightOverride(CircleButtonSize)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(Label))
                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
                    .ColorAndOpacity(PhyControlColors::TextWhite)
                ]
            ]
        ];
}

TSharedRef<SWidget> UPhyControl_UI::CreateBottomButton(
    const FString& Label,
    TSharedPtr<SButton>& OutButton,
    TFunction<void()> OnClicked)
{
    using namespace PhyControlSizes;
    
    return SAssignNew(OutButton, SButton)
        .ButtonColorAndOpacity(PhyControlColors::ButtonOrange)
        .OnClicked_Lambda([OnClicked]()
        { 
            if (OnClicked) OnClicked();
            return FReply::Handled(); 
        })
        [
            SNew(SBox)
            .WidthOverride(BottomButtonWidth)
            .HeightOverride(BottomButtonHeight)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Label))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
                .ColorAndOpacity(PhyControlColors::TextWhite)
            ]
        ];
}

// ==================== 自定义滑块 ====================

TSharedRef<SWidget> UPhyControl_UI::CreateCustomVerticalSlider(
    const FString& Label,
    float* ValuePtr,
    TFunction<void(float)> OnChanged)
{
    using namespace PhyControlSizes;
    
    // 使用 SOverlay 组合：轨道 + 滑块
    // 滑块是横向的长方形
    
    return SNew(SVerticalBox)
        
        // 滑块区域
        + SVerticalBox::Slot()
        .AutoHeight()
        .HAlign(HAlign_Center)
        [
            SNew(SBox)
            .WidthOverride(SliderThumbWidth + 10)
            .HeightOverride(SliderTrackHeight)
            [
                SNew(SOverlay)
                
                // 轨道背景 (竖线)
                + SOverlay::Slot()
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Fill)
                [
                    SNew(SBox)
                    .WidthOverride(SliderTrackWidth)
                    [
                        SNew(SBorder)
                        .BorderBackgroundColor(PhyControlColors::SliderTrack)
                    ]
                ]
                
                // 滑块控件
                + SOverlay::Slot()
                [
                    SNew(SSlider)
                    .Orientation(Orient_Vertical)
                    .Value(ValuePtr ? *ValuePtr : 0.5f)
                    .SliderBarColor(FLinearColor::Transparent)  // 隐藏默认轨道
                    .OnValueChanged_Lambda([ValuePtr, OnChanged](float NewValue)
                    {
                        if (ValuePtr) *ValuePtr = NewValue;
                        if (OnChanged) OnChanged(NewValue);
                    })
                ]
            ]
        ]
        
        // 标签
        + SVerticalBox::Slot()
        .AutoHeight()
        .HAlign(HAlign_Center)
        .Padding(0, 10, 0, 0)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Label))
            .Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
            .ColorAndOpacity(PhyControlColors::TextWhite)
        ];
}

TSharedRef<SWidget> UPhyControl_UI::CreateCustomHorizontalSlider(
    float* ValuePtr,
    float MinValue,
    float MaxValue,
    TFunction<void(float)> OnChanged)
{
    using namespace PhyControlSizes;
    
    float NormalizedValue = ValuePtr ? (*ValuePtr - MinValue) / (MaxValue - MinValue) : 0.5f;
    
    return SNew(SBox)
        .HeightOverride(20)
        [
            SNew(SOverlay)
            
            // 轨道背景
            + SOverlay::Slot()
            .VAlign(VAlign_Center)
            [
                SNew(SBox)
                .HeightOverride(SliderTrackWidth)
                [
                    SNew(SBorder)
                    .BorderBackgroundColor(PhyControlColors::SliderTrack)
                ]
            ]
            
            // 滑块
            + SOverlay::Slot()
            [
                SNew(SSlider)
                .Orientation(Orient_Horizontal)
                .Value(NormalizedValue)
                .SliderBarColor(FLinearColor::Transparent)
                .OnValueChanged_Lambda([ValuePtr, MinValue, MaxValue, OnChanged](float NormValue)
                {
                    float ActualValue = MinValue + NormValue * (MaxValue - MinValue);
                    if (ValuePtr) *ValuePtr = ActualValue;
                    if (OnChanged) OnChanged(ActualValue);
                })
            ]
        ];
}

// ==================== 回调 ====================

void UPhyControl_UI::OnWorldToggle()
{
    OnBoolChanged.Broadcast(FName("WorldEnabled"), ControlData.WorldSpace.bEnabled);
    NotifyDataChanged();
}

void UPhyControl_UI::OnParentToggle()
{
    OnBoolChanged.Broadcast(FName("ParentEnabled"), ControlData.ParentSpace.bEnabled);
    NotifyDataChanged();
}

void UPhyControl_UI::NotifyDataChanged()
{
    OnDataChanged.Broadcast(ControlData);
}

void UPhyControl_UI::OnResetRequested()
{
    ControlData.Options.bResetRequested = true;
    NotifyDataChanged();
    ControlData.Options.bResetRequested = false;
}

void UPhyControl_UI::OnInitializeRequested()
{
    ControlData.Options.bInitializeRequested = true;
    NotifyDataChanged();
    ControlData.Options.bInitializeRequested = false;
}