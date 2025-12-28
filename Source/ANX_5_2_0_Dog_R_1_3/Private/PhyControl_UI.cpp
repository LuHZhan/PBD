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
	constexpr FLinearColor Background(0.1f, 0.1f, 0.1f, 0.85f);
	constexpr FLinearColor PanelBorder(0.2f, 0.2f, 0.2f, 1.0f);
	constexpr FLinearColor InactiveGray(0.4f, 0.4f, 0.4f, 1.0f);
	constexpr FLinearColor SliderTrack(0.15f, 0.15f, 0.15f, 1.0f);
	constexpr FLinearColor SliderThumb(0.7f, 0.7f, 0.7f, 1.0f);
	constexpr FLinearColor ButtonOrange(0.7f, 0.8f, 0.1f, 1.0f);
	constexpr FLinearColor TextWhite(1.0f, 1.0f, 1.0f, 1.0f);
	constexpr FLinearColor ButtonDefaultGreen(0.3f, 0.85f, 0.3f, 0.5f); // 淡蓝色（默认）
	constexpr FLinearColor ButtonClickedRed(1.0f, 0.7f, 0.7f, 0.5f); // 淡红色（点击后）

	constexpr FLinearColor ActiveGreen(0.3f, 0.85f, 0.3f, 0.5f);
}

// ==================== 尺寸定义 ====================
namespace PhyControlSizes
{
	constexpr float PanelWidth = 500.0f;
	constexpr float PanelHeight = 300.0f;
	constexpr float SliderTrackHeight = 120.0f;
	constexpr float SliderTrackWidth = 4.0f;
	constexpr float SliderThumbWidth = 300.0f;
	constexpr float SliderThumbHeight = 8.0f;
	constexpr float CircleButtonSize = 28.0f;

	constexpr float BottomButtonWidth = 30.0f;
	constexpr float BottomButtonHeight = 30.0f;
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
	WorldStrengthSlider.Reset();
	WorldDampingSlider.Reset();
	ParentStrengthSlider.Reset();
	ParentDampingSlider.Reset();
}

void UPhyControl_UI::NativePreConstruct()
{
	Super::NativePreConstruct();
}

// ==================== UI 更新 ====================

void UPhyControl_UI::UpdateIndicator(TSharedPtr<SImage>& Indicator, bool bEnabled)
{
	if (Indicator.IsValid())
	{
		// Indicator->SetImage(&GetIndicatorBrush(bEnabled));
		UpdateIndicatorBrush(Indicator, bEnabled);
		// Indicator->SetImage(&(bEnabled ? EnabledBrush : DisabledBrush));
	}
}

void UPhyControl_UI::UpdateCircleButton(TSharedPtr<SBorder>& Indicator, bool bEnabled)
{
	if (Indicator.IsValid())
	{
		Indicator->SetBorderBackgroundColor(
			bEnabled ? PhyControlColors::ButtonClickedRed : PhyControlColors::ButtonDefaultGreen
		);
	}
}

void UPhyControl_UI::InitializeUIFromData()
{
	if (WorldIndicator && ParentIndicator)
	{
		UpdateIndicator(WorldIndicator, ControlData.WorldSpace.bEnabled);
		UpdateIndicator(ParentIndicator, ControlData.ParentSpace.bEnabled);
	}

	if (ButtonP_Indicator && ButtonG_Indicator && ButtonS_Indicator && ButtonM_Indicator && ButtonB_Indicator)
	{
		UpdateCircleButton(ButtonP_Indicator, ControlData.BodyModifier.bPhysicsEnabled);
		UpdateCircleButton(ButtonG_Indicator, ControlData.BodyModifier.bGravityEnabled);
		UpdateCircleButton(ButtonS_Indicator, ControlData.BodyModifier.bSimulated);

		// 一下两个都是数值，TODO
		UpdateCircleButton(ButtonM_Indicator, ControlData.BodyModifier.Multiplier > 1.0f);
		UpdateCircleButton(ButtonB_Indicator, ControlData.BodyModifier.BlendWeight > 0.5f);
	}

	if (WorldStrengthSlider && WorldDampingSlider && ParentStrengthSlider && ParentDampingSlider)
	{
		WorldStrengthSlider->SetValue(TAttribute<float>(ControlData.WorldSpace.Strength));
		WorldDampingSlider->SetValue(TAttribute<float>(ControlData.WorldSpace.Damping));
		ParentStrengthSlider->SetValue(TAttribute<float>(ControlData.ParentSpace.Strength));
		ParentDampingSlider->SetValue(TAttribute<float>(ControlData.WorldSpace.Damping));
	}
}

// ==================== 蓝图可重载默认实现 ====================

void UPhyControl_UI::UpdateIndicatorBrush(TSharedPtr<SImage>& Indicator, bool bEnabled)
{
	FSlateBrush Brush = bEnabled ? EnabledBrush : DisabledBrush;
	Brush.TintColor = FSlateColor(bEnabled ? PhyControlColors::ActiveGreen : PhyControlColors::InactiveGray);
	if (Indicator)
	{
		Indicator->SetImage(&(bEnabled ? EnabledBrush : DisabledBrush));
	}
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
				.Visibility(EVisibility::SelfHitTestInvisible)
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
								EPhyControlSpace::World,
								L"World",
								&ControlData.WorldSpace,
								&ControlData.ParentSpace, WorldIndicator, [this]() { OnWorldToggle(); }
							)
						]

						// Parent 组
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(15, 0, 15, 0)
						[
							CreateControlSpacePanel(
								EPhyControlSpace::Parent,
								L"Parent",
								&ControlData.WorldSpace,
								&ControlData.ParentSpace, ParentIndicator, [this]() { OnParentToggle(); }
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

	InitializeUIFromData();
	return RootWidget.ToSharedRef();
}

// ==================== 控制空间面板 ====================

TSharedRef<SWidget> UPhyControl_UI::CreateControlSpacePanel(
	EPhyControlSpace SpaceType,
	const FString& Title,
	FPhyControlSpaceData* WorldSpace,
	FPhyControlSpaceData* ParentSpace, TSharedPtr<SImage>& OutIndicator, TFunction<void()> OnToggled)
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
					CreateIndicatorButton(SpaceType, &WorldSpace->bEnabled, &ParentSpace->bEnabled, OutIndicator, OnToggled)
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
					CreateCustomVerticalSlider(TEXT("Strength"),
					                           SpaceType == EPhyControlSpace::World ? &WorldSpace->Strength : &ParentSpace->Strength,
					                           SpaceType == EPhyControlSpace::World ? WorldStrengthSlider : ParentStrengthSlider,
					                           [this](float V) { NotifyDataChanged(); })
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Center)
				[
					CreateCustomVerticalSlider(TEXT("Damping"),
					                           SpaceType == EPhyControlSpace::World ? &WorldSpace->Damping : &ParentSpace->Damping,
					                           SpaceType == EPhyControlSpace::World ? WorldDampingSlider : ParentDampingSlider,
					                           [this](float V) { NotifyDataChanged(); })
				]
			];
}

// ==================== 右侧按钮列 ====================

TSharedRef<SWidget> UPhyControl_UI::CreateRightButtonColumn()
{
	float BtnBottomPadding = 10.0f;
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, BtnBottomPadding)
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
		.Padding(0, 0, 0, BtnBottomPadding)
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
		.Padding(0, 0, 0, BtnBottomPadding)
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
		.Padding(0, 0, 0, BtnBottomPadding)
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
	EPhyControlSpace SpaceType,
	bool* WorldSpaceEnabledPtr,
	bool* ParentSpaceEnabledPtr, TSharedPtr<SImage>& OutIndicator, TFunction<void()> OnClicked)
{
	bool bInitialEnabled = WorldSpaceEnabledPtr && *WorldSpaceEnabledPtr;

	UpdateIndicatorBrush(OutIndicator, bInitialEnabled);

	return SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), "NoBorder")
		.OnClicked_Lambda([this, SpaceType,ParentSpaceEnabledPtr,WorldSpaceEnabledPtr, &OutIndicator, OnClicked]()
		{
			if (WorldSpaceEnabledPtr && ParentSpaceEnabledPtr)
			{
				*WorldSpaceEnabledPtr = !(*WorldSpaceEnabledPtr);
				*ParentSpaceEnabledPtr = !(*ParentSpaceEnabledPtr);

				if (WorldIndicator && ParentIndicator)
				{
					UpdateIndicator(WorldIndicator, *WorldSpaceEnabledPtr);
					UpdateIndicator(ParentIndicator, *ParentSpaceEnabledPtr);
				}
			}

			if (OnClicked) OnClicked();
			return FReply::Handled();
		})
		[
			SAssignNew(OutIndicator, SImage)
			.DesiredSizeOverride(FVector2D(24, 24))
			.Image(&(bInitialEnabled ? EnabledBrush : DisabledBrush))
		];
}

// 创建圆形按钮（右侧按钮列：P/G/S/M/B）
// 实现方式：使用 SBorder 包裹 SButton，通过 BorderBackgroundColor 控制背景色填充
// 优点：可以动态改变背景色（点击后变红），完全填充背景，显示效果一致
TSharedRef<SWidget> UPhyControl_UI::CreateCircleButton(
	const FString& Label,
	bool* EnabledPtr,
	TSharedPtr<SBorder>& OutIndicator,
	TFunction<void()> OnClicked)
{
	using namespace PhyControlSizes;

	return SNew(SBox)
		.WidthOverride(45.0f)
		.HeightOverride(30.0f)
		[
			SAssignNew(OutIndicator, SBorder)
			                                 .Visibility(EVisibility::SelfHitTestInvisible)
			                                 .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")) // 使用白色填充Brush实现完全填充背景
			                                 .BorderBackgroundColor(PhyControlColors::ButtonDefaultGreen) // 默认淡绿色背景
			                                 .Padding(0)
			[
				SNew(SButton)
				             .ButtonStyle(FAppStyle::Get(), "NoBorder")
				             .ContentPadding(FMargin(0))
				// 注意：使用 &OutIndicator 引用捕获，确保每个按钮的 Lambda 都能正确更新自己的 OutIndicator
				// 如果使用值捕获 [OutIndicator]，所有按钮可能会共享同一个 OutIndicator 引用
				             .OnClicked_Lambda([OnClicked, &OutIndicator]()
				             {
					             // // 点击后更新背景色为淡红色
					             // if (OutIndicator.IsValid())
					             // {
					             //  OutIndicator->SetBorderBackgroundColor(PhyControlColors::ButtonClickedRed);
					             // }
					             if (OnClicked) OnClicked();
					             return FReply::Handled();
				             })
				[
					SNew(SBox)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Visibility(EVisibility::SelfHitTestInvisible)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Label))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(PhyControlColors::TextWhite)
						.Visibility(EVisibility::SelfHitTestInvisible)
					]
				]
			]
		];
}

// 创建底部按钮（R/I/W/S）
// 实现方式：使用 SBorder 包裹 SButton，通过 BorderBackgroundColor 控制背景色填充
// 与 CreateCircleButton 使用相同的实现方式，确保显示效果一致
TSharedRef<SWidget> UPhyControl_UI::CreateBottomButton(
	const FString& Label,
	TSharedPtr<SButton>& OutButton,
	TFunction<void()> OnClicked)
{
	using namespace PhyControlSizes;

	return SAssignNew(OutButton, SButton)
		.ButtonColorAndOpacity(PhyControlColors::ButtonDefaultGreen)
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
	TSharedPtr<SSlider>& OutSlider,
	TFunction<void(float)> OnChanged)
{
	// 创建横向长方形 Brush
	FSlateBrush ThumbBrush;
	ThumbBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
	ThumbBrush.TintColor = FSlateColor(PhyControlColors::TextWhite);
	ThumbBrush.ImageSize = FVector2D(20.0f, 20.0f);

	CustomSliderThumbStyle.SetNormalThumbImage(ThumbBrush);
	CustomSliderThumbStyle.SetHoveredThumbImage(ThumbBrush);
	CustomSliderThumbStyle.SetDisabledThumbImage(ThumbBrush);
	// CustomSliderStyle.SetBarThickness(4.0f);

	// 滑块尺寸
	const float TrackHeight = 120.0f;
	const float TrackWidth = 4.0f;

	return SNew(SVerticalBox)

			// 滑块区域
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(TrackWidth + 10)
				.HeightOverride(TrackHeight)
				[
					SNew(SOverlay)

					// 轨道背景 (竖线)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Fill)
					[
						SNew(SBox)
						.WidthOverride(TrackWidth * 1.5)
						[
							SNew(SImage)
							.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.ColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f))
						]
					]

					// 使用 SSlider，但设置 IndentHandle=false 让滑块填满
					+ SOverlay::Slot()
					[
						SAssignNew(OutSlider, SSlider)
						                              .Style(&CustomSliderThumbStyle)
						                              .Orientation(Orient_Vertical)
						                              .IndentHandle(false) // 关键：让滑块可以到达边缘
						                              .Value(ValuePtr ? *ValuePtr : 0.5f)
						                              .SliderBarColor(FLinearColor::Transparent)
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
				.ColorAndOpacity(FLinearColor::White)
			];
}

TSharedRef<SWidget> UPhyControl_UI::CreateCustomHorizontalSlider(
	float* ValuePtr,
	float MinValue,
	float MaxValue,
	TFunction<void(float)> OnChanged)
{
	using namespace PhyControlSizes;

	// float NormalizedValue = ValuePtr ? (*ValuePtr - MinValue) / (MaxValue - MinValue) : 0.5f;

	return SNew(SBox)
		.HeightOverride(6)
		[
			SNew(SOverlay)

			// 轨道背景 (横线)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				// .HeightOverride(2)
				[
					SNew(SImage)
					.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.ColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f))
				]
			]

			// 滑块
			+ SOverlay::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(SSlider)
				             .Style(&CustomSliderThumbStyle)
				             .Orientation(Orient_Horizontal)
				             .IndentHandle(false) // 关键：让滑块可以到达边缘
				             .Value(ValuePtr ? *ValuePtr : 0.5f)
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
