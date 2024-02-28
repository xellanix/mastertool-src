// msgbx.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "msgbx.h"
#include <shellapi.h>

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.system.h>
#include <winrt/windows.ui.xaml.hosting.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
#include <winrt/windows.ui.xaml.controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.ui.xaml.media.h>
#include <winrt/windows.ui.xaml.markup.h>

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND _hWnd;
DesktopWindowXamlSource desktopXamlSource{ nullptr };

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MSGBX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    hInst = hInstance; // Store instance handle in our global variable

    MONITORINFO info{};
    {
        info.cbSize = sizeof(MONITORINFO);

        const POINT ptZero = { 0, 0 };
        auto monitor = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);

        if (GetMonitorInfo(monitor, &info) == FALSE)
            return FALSE;

        const auto windowWidth = 390, windowHeight = 247;
        const auto windowx = (info.rcWork.right - windowWidth) / 2;
        const auto windowy = (info.rcWork.bottom - windowHeight) / 2;
        _hWnd = CreateWindowExW(WS_EX_TOPMOST, szWindowClass, szTitle, WS_POPUP,
                                windowx, windowy, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);
    }

    if (!_hWnd)
    {
        return FALSE;
    }
    {
        auto iDpi = GetDpiForWindow(_hWnd);
        auto scale = (double)iDpi / 96.0;

        const auto windowWidth = (int)std::ceil(390.0 * scale), windowHeight = (int)std::ceil(247 * scale);
        const auto windowx = (info.rcWork.right - windowWidth) / 2;
        const auto windowy = (info.rcWork.bottom - windowHeight) / 2;
        const auto roundSize = (int)std::ceil(16.0 * scale);
        const auto hrgn = CreateRoundRectRgn(0, 0, windowWidth + 1, windowHeight + 1, roundSize, roundSize);

        SetWindowPos(_hWnd, NULL, windowx, windowy, windowWidth, windowHeight, SWP_SHOWWINDOW);
        SetWindowRgn(_hWnd, hrgn, TRUE);
        
        DeleteObject(hrgn);
    }

    winrt::init_apartment(apartment_type::single_threaded);

    WindowsXamlManager winxamlmanager = WindowsXamlManager::InitializeForCurrentThread();
    
    desktopXamlSource = DesktopWindowXamlSource{};
    if (desktopXamlSource)
    {
        auto interop = desktopXamlSource.as<IDesktopWindowXamlSourceNative>();
        check_hresult(interop->AttachToWindow(_hWnd));

        // Get the new child window's hwnd
        HWND hWndXamlIsland = nullptr;
        check_hresult(interop->get_WindowHandle(&hWndXamlIsland));
        RECT clientRc{};
        if (GetClientRect(_hWnd, &clientRc))
            SetWindowPos(hWndXamlIsland, nullptr, 0, 0, clientRc.right, clientRc.bottom, SWP_SHOWWINDOW);

        Grid grid{};
        {
            grid = Markup::XamlReader::Load(LR"(<Grid xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
    <Grid.Resources>
        <ResourceDictionary>
            <ResourceDictionary.ThemeDictionaries>
                <ResourceDictionary x:Key="Default">
                    <SolidColorBrush x:Key="ContentDialogBackground" Color="#202020" />
                    <SolidColorBrush x:Key="ContentDialogTopOverlay" Color="#0DFFFFFF" />
                    <SolidColorBrush x:Key="ContentDialogBorderBrush" Color="#66757575" />
                    <SolidColorBrush x:Key="ContentDialogSeparatorBorderBrush" Color="#19000000" />

                    <SolidColorBrush x:Key="AccentButtonBackground" Color="#FF4CC2FF" />
                    <SolidColorBrush x:Key="AccentButtonBackgroundPointerOver" Color="#FF4CC2FF" Opacity="0.9" />
                    <SolidColorBrush x:Key="AccentButtonBackgroundPressed" Color="#FF0067C0" Opacity="0.8" />
                    <SolidColorBrush x:Key="AccentButtonBackgroundDisabled" Color="#28FFFFFF" />
                    <SolidColorBrush x:Key="AccentButtonForeground" Color="#000000" />
                    <SolidColorBrush x:Key="AccentButtonForegroundPointerOver" Color="#000000" />
                    <SolidColorBrush x:Key="AccentButtonForegroundPressed" Color="#80000000" />
                    <SolidColorBrush x:Key="AccentButtonForegroundDisabled" Color="#87FFFFFF" />
            
                    <LinearGradientBrush x:Key="AccentButtonBorderBrush" MappingMode="Absolute" StartPoint="0,0" EndPoint="0,3">
                        <LinearGradientBrush.RelativeTransform>
                            <ScaleTransform ScaleY="-1" CenterY="0.5"/>
                        </LinearGradientBrush.RelativeTransform>
                        <LinearGradientBrush.GradientStops>
                            <GradientStop Offset="0.33" Color="#23000000"/>
                            <GradientStop Offset="1.0" Color="#14FFFFFF"/>
                        </LinearGradientBrush.GradientStops>
                    </LinearGradientBrush>
                    <LinearGradientBrush x:Key="AccentButtonBorderBrushPointerOver" MappingMode="Absolute" StartPoint="0,0" EndPoint="0,3">
                        <LinearGradientBrush.RelativeTransform>
                            <ScaleTransform ScaleY="-1" CenterY="0.5"/>
                        </LinearGradientBrush.RelativeTransform>
                        <LinearGradientBrush.GradientStops>
                            <GradientStop Offset="0.33" Color="#23000000"/>
                            <GradientStop Offset="1.0" Color="#14FFFFFF"/>
                        </LinearGradientBrush.GradientStops>
                    </LinearGradientBrush>

                    <SolidColorBrush x:Key="AccentButtonBorderBrushPressed" Color="#00FFFFFF" />
                    <SolidColorBrush x:Key="AccentButtonBorderBrushDisabled" Color="#00FFFFFF" />
                </ResourceDictionary>
                <ResourceDictionary x:Key="HighContrast">
                    <SolidColorBrush x:Key="ContentDialogBackground" Color="{ThemeResource SystemColorWindowColor}" />
                    <SolidColorBrush x:Key="ContentDialogTopOverlay" Color="{ThemeResource SystemControlTransparent}" />
                    <SolidColorBrush x:Key="ContentDialogBorderBrush" Color="{ThemeResource SystemColorWindowTextColor}" />
                    <SolidColorBrush x:Key="ContentDialogSeparatorBorderBrush" Color="{ThemeResource SystemColorWindowTextColor}" />

                    <SolidColorBrush x:Key="AccentButtonBackground" Color="{ThemeResource SystemControlHighlightAccent}" />
                    <SolidColorBrush x:Key="AccentButtonBackgroundPointerOver" Color="{ThemeResource SystemControlForegroundAccent}" />
                    <SolidColorBrush x:Key="AccentButtonBackgroundPressed" Color="{ThemeResource SystemControlBackgroundBaseMediumLow}" />
                    <SolidColorBrush x:Key="AccentButtonBackgroundDisabled" Color="{ThemeResource SystemControlBackgroundBaseLow}" />
                    <SolidColorBrush x:Key="AccentButtonForeground" Color="{ThemeResource SystemControlHighlightAltAccent}" />
                    <SolidColorBrush x:Key="AccentButtonForegroundPointerOver" Color="{ThemeResource SystemControlBackgroundChromeWhite}" />
                    <SolidColorBrush x:Key="AccentButtonForegroundPressed" Color="{ThemeResource SystemControlHighlightBaseHigh}" />
                    <SolidColorBrush x:Key="AccentButtonForegroundDisabled" Color="{ThemeResource SystemControlDisabledBaseMediumLow}" />
                    <SolidColorBrush x:Key="AccentButtonBorderBrush" Color="{ThemeResource SystemControlHighlightAccent}" />
                    <SolidColorBrush x:Key="AccentButtonBorderBrushPointerOver" Color="{ThemeResource SystemControlHighlightBaseMediumLow}" />
                    <SolidColorBrush x:Key="AccentButtonBorderBrushPressed" Color="{ThemeResource SystemControlHighlightTransparent}" />
                    <SolidColorBrush x:Key="AccentButtonBorderBrushDisabled" Color="{ThemeResource SystemControlDisabledTransparent}" />
                </ResourceDictionary>)" LR"(
                <ResourceDictionary x:Key="Light">
                    <SolidColorBrush x:Key="ContentDialogBackground" Color="#F3F3F3" />
                    <SolidColorBrush x:Key="ContentDialogTopOverlay" Color="#FFFFFF" />
                    <SolidColorBrush x:Key="ContentDialogBorderBrush" Color="#66757575" />
                    <SolidColorBrush x:Key="ContentDialogSeparatorBorderBrush" Color="#0F000000" />

                    <SolidColorBrush x:Key="AccentButtonBackground" Color="#FF0067C0" />
                    <SolidColorBrush x:Key="AccentButtonBackgroundPointerOver" Color="#FF0067C0" Opacity="0.9" />
                    <SolidColorBrush x:Key="AccentButtonBackgroundPressed" Color="#FF0067C0" Opacity="0.8" />
                    <SolidColorBrush x:Key="AccentButtonBackgroundDisabled" Color="#37000000" />
                    <SolidColorBrush x:Key="AccentButtonForeground" Color="#FFFFFF" />
                    <SolidColorBrush x:Key="AccentButtonForegroundPointerOver" Color="#FFFFFF" />
                    <SolidColorBrush x:Key="AccentButtonForegroundPressed" Color="#B3FFFFFF" />
                    <SolidColorBrush x:Key="AccentButtonForegroundDisabled" Color="#FFFFFF" />
                    
                    <LinearGradientBrush x:Key="AccentButtonBorderBrush" MappingMode="Absolute" StartPoint="0,0" EndPoint="0,3">
                        <LinearGradientBrush.RelativeTransform>
                            <ScaleTransform ScaleY="-1" CenterY="0.5"/>
                        </LinearGradientBrush.RelativeTransform>
                        <LinearGradientBrush.GradientStops>
                            <GradientStop Offset="0.33" Color="#66000000"/>
                            <GradientStop Offset="1.0" Color="#14FFFFFF"/>
                        </LinearGradientBrush.GradientStops>
                    </LinearGradientBrush>
                    <LinearGradientBrush x:Key="AccentButtonBorderBrushPointerOver" MappingMode="Absolute" StartPoint="0,0" EndPoint="0,3">
                        <LinearGradientBrush.RelativeTransform>
                            <ScaleTransform ScaleY="-1" CenterY="0.5"/>
                        </LinearGradientBrush.RelativeTransform>
                        <LinearGradientBrush.GradientStops>
                            <GradientStop Offset="0.33" Color="#66000000"/>
                            <GradientStop Offset="1.0" Color="#14FFFFFF"/>
                        </LinearGradientBrush.GradientStops>
                    </LinearGradientBrush>

                    <SolidColorBrush x:Key="AccentButtonBorderBrushPressed" Color="#00FFFFFF" />
                    <SolidColorBrush x:Key="AccentButtonBorderBrushDisabled" Color="#00FFFFFF" />
                </ResourceDictionary>
            </ResourceDictionary.ThemeDictionaries>
        </ResourceDictionary>
    </Grid.Resources>
    <Border
        x:Name="BackgroundElement"
        Background="{ThemeResource ContentDialogBackground}"
        BorderThickness="1"
        BorderBrush="{ThemeResource ContentDialogBorderBrush}"
        BackgroundSizing="InnerBorderEdge"
        CornerRadius="8"
        Width="390"
        Height="247"
        HorizontalAlignment="Center"
        VerticalAlignment="Center"
        RenderTransformOrigin="0.5,0.5">
        <Border.RenderTransform>
            <ScaleTransform x:Name="ScaleTransform" />
        </Border.RenderTransform>
        <Grid x:Name="DialogSpace" CornerRadius="8">
            <Grid.RowDefinitions>
                <RowDefinition Height="*" />
                <RowDefinition Height="Auto" />
            </Grid.RowDefinitions>
            <ScrollViewer
                x:Name="ContentScrollViewer"
                HorizontalScrollBarVisibility="Disabled"
                VerticalScrollBarVisibility="Disabled"
                ZoomMode="Disabled"
                IsTabStop="False">
                <Grid
                    Background="{ThemeResource ContentDialogTopOverlay}"
                    Padding="24"
                    BorderThickness="0,0,0,1"
                    BorderBrush="{ThemeResource ContentDialogSeparatorBorderBrush}">
                    <Grid.RowDefinitions>
                        <RowDefinition Height="Auto" />
                        <RowDefinition Height="Auto" />
                        <RowDefinition Height="*" />
                    </Grid.RowDefinitions>
                    <Grid Margin="0,0,0,12" ColumnSpacing="12">
                        <Grid.ColumnDefinitions>
                            <ColumnDefinition Width="Auto"/>
                            <ColumnDefinition/>
                        </Grid.ColumnDefinitions>
                        <Viewbox Width="16" Height="16" HorizontalAlignment="Left" VerticalAlignment="Center">
                            <Canvas Height="256" Width="256">
                                <Path Fill="#1A000000" Data="M31.76,243c-12,0-21.76-9.96-21.76-22.2v-87.13c-4.72-2.01-8-6.76-8-12.25V80.06C2,64.56,14.56,52,30.06,52  H65v-6.75C65,26.89,79.89,12,98.25,12h58.5C175.11,12,190,26.89,190,45.25V52h35.94C241.44,52,254,64.56,254,80.06v41.35  c0,5.49-3.28,10.24-8,12.25v87.13c0,12.24-9.76,22.2-21.76,22.2H31.76z M154,52v-3h-53v3H154z" />
                                <Path Fill="#14000000" Data="M32.19,239C22.71,239,15,231.11,15,221.42v-90.94c-4.45-0.24-8-4.02-8-8.62V80.39C7,67.47,17.47,57,30.39,57  H69V45.58C69,29.8,81.8,17,97.58,17h58.83C172.2,17,185,29.8,185,45.58V57h40.61C238.53,57,249,67.47,249,80.39v41.48  c0,4.6-3.55,8.38-8,8.62v90.94c0,9.69-7.71,17.58-17.19,17.58H32.19z M158,57V45.75c0-0.97-0.78-1.75-1.75-1.75h-58.5  C96.78,44,96,44.78,96,45.75V57H158z" />
                                <Path Fill="#4CC2FF" Data="M237,126v95.09c0,7.1-5.81,12.91-12.91,12.91H32.88c-7.1,0-12.88-5.77-12.88-12.85V126H237z" />
                                <Rectangle Canvas.Left="20" Canvas.Top="122" Fill="#2BA6FF" Width="217" Height="14" />
                                <Path Fill="#36AFFF" Data="M89,136v8.53c0,4.11-3.36,7.47-7.47,7.47H52.47c-4.13,0-7.47-3.34-7.47-7.47V136H89z" />
                                <Path Fill="#1C8FFF" Data="M89,105.46V136H45v-30.53c0-4.13,3.34-7.47,7.47-7.47h29.07C85.66,98,89,101.34,89,105.46z" />
                                <Path Fill="#36AFFF" Data="M212,136v8.53c0,4.13-3.34,7.47-7.47,7.47h-29.06c-4.13,0-7.47-3.34-7.47-7.47V136H212z" />
                                <Path Fill="#1C8FFF" Data="M212,105.47V136h-44v-30.53c0-4.13,3.34-7.47,7.47-7.47h29.06C208.66,98,212,101.34,212,105.47z" />
                                <Path Fill="#0091F8" Data="M156.08,22H97.92C84.71,22,74,32.71,74,45.92V61h18V45.42c0-3.54,2.87-6.42,6.42-6.42h58.17  c3.54,0,6.42,2.87,6.42,6.42V61h17V45.92C180,32.71,169.29,22,156.08,22z" />
                                <Path Fill="#E6E6E6" Data="M245,80.72v41.31c0,2.19-1.78,3.97-3.97,3.97H15.97c-2.19,0-3.97-1.78-3.97-3.97V80.72  C12,70.38,20.38,62,30.72,62h195.55C236.62,62,245,70.38,245,80.72z" />
                                <Path Fill="#0091F8" Data="M204.03,101h-29.07c-2.19,0-3.97,1.78-3.97,3.97v39.07c0,2.19,1.78,3.97,3.97,3.97h29.07  c2.19,0,3.97-1.78,3.97-3.97v-39.07C208,102.78,206.22,101,204.03,101z" />
                                <Path Fill="#0091F8" Data="M82.03,101H52.97c-2.19,0-3.97,1.78-3.97,3.97v39.07c0,2.19,1.78,3.97,3.97,3.97h29.08  c2.18,0,3.96-1.77,3.96-3.96v-39.08C86,102.78,84.22,101,82.03,101z" />
                            </Canvas>
                        </Viewbox>
                        <TextBlock x:Name="AppTitle" FontSize="12" Grid.Column="1" VerticalAlignment="Center"/>
                    </Grid>)" LR"(
                    <ContentControl
                        x:Name="Title"
                        Margin="0,0,0,12"
                        FontSize="20"
                        FontFamily="{StaticResource ContentControlThemeFontFamily}"
                        FontWeight="SemiBold"
                        HorizontalAlignment="Left"
                        VerticalAlignment="Top"
                        IsTabStop="False"
                        Grid.Row="1">
                        <ContentControl.Template>
                            <ControlTemplate TargetType="ContentControl">
                                <ContentPresenter
                                    Content="{TemplateBinding Content}"
                                    MaxLines="2"
                                    TextWrapping="Wrap"
                                    ContentTemplate="{TemplateBinding ContentTemplate}"
                                    Margin="{TemplateBinding Padding}"
                                    ContentTransitions="{TemplateBinding ContentTransitions}"
                                    HorizontalAlignment="{TemplateBinding HorizontalContentAlignment}"
                                    VerticalAlignment="{TemplateBinding VerticalContentAlignment}" />
                            </ControlTemplate>
                        </ContentControl.Template>
                    </ContentControl>
                    <TextBlock
                        x:Name="Content"
                        FontSize="14"
                        Grid.Row="2"
                        TextWrapping="Wrap" />
                </Grid>
            </ScrollViewer>
            <Grid
                x:Name="CommandSpace"
                Grid.Row="1"
                HorizontalAlignment="Stretch"
                VerticalAlignment="Bottom"
                XYFocusKeyboardNavigation="Enabled"
                Padding="20"
                Background="{ThemeResource ContentDialogBackground}">
                <Grid.ColumnDefinitions>
                    <ColumnDefinition Width="*" />
                    <ColumnDefinition Width="*" />
                </Grid.ColumnDefinitions>
                <Button
                    x:Name="CloseButton"
                    IsTabStop="False"
                    Grid.Column="1" 
                    Content="Close"
                    ElementSoundMode="FocusOnly"
                    HorizontalAlignment="Stretch">
                    <Button.Style>
                        <Style x:Key="AccentButtonStyle" TargetType="Button">
                            <Setter Property="Foreground" Value="{ThemeResource AccentButtonForeground}" />
                            <Setter Property="Background" Value="{ThemeResource AccentButtonBackground}" />
                            <Setter Property="BackgroundSizing" Value="OuterBorderEdge" />
                            <Setter Property="BorderBrush" Value="{ThemeResource AccentButtonBorderBrush}" />
                            <Setter Property="CornerRadius" Value="4" />
                            <Setter Property="Template">
                                <Setter.Value>
                                    <ControlTemplate TargetType="Button">
                                        <ContentPresenter
                                            x:Name="ContentPresenter"
                                            Background="{TemplateBinding Background}"
                                            BackgroundSizing="{TemplateBinding BackgroundSizing}"
                                            Foreground="{TemplateBinding Foreground}"
                                            BorderBrush="{TemplateBinding BorderBrush}"
                                            BorderThickness="{TemplateBinding BorderThickness}"
                                            Content="{TemplateBinding Content}"
                                            ContentTemplate="{TemplateBinding ContentTemplate}"
                                            ContentTransitions="{TemplateBinding ContentTransitions}"
                                            CornerRadius="{TemplateBinding CornerRadius}"
                                            Padding="{TemplateBinding Padding}"
                                            HorizontalContentAlignment="{TemplateBinding HorizontalContentAlignment}"
                                            VerticalContentAlignment="{TemplateBinding VerticalContentAlignment}"
                                            AutomationProperties.AccessibilityView="Raw">

                                            <ContentPresenter.BackgroundTransition>
                                                <BrushTransition Duration="0:0:0.083" />
                                            </ContentPresenter.BackgroundTransition>
                                            
                                            <VisualStateManager.VisualStateGroups>
                                                <VisualStateGroup x:Name="CommonStates">
                                                    <VisualState x:Name="Normal"/>
                                                    <VisualState x:Name="PointerOver">
                                                        <Storyboard>
                                                            <ObjectAnimationUsingKeyFrames Storyboard.TargetName="ContentPresenter" Storyboard.TargetProperty="Background">
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource AccentButtonBackgroundPointerOver}" />
                                                            </ObjectAnimationUsingKeyFrames>
                                                            <ObjectAnimationUsingKeyFrames Storyboard.TargetName="ContentPresenter" Storyboard.TargetProperty="BorderBrush">
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource AccentButtonBorderBrushPointerOver}" />
                                                            </ObjectAnimationUsingKeyFrames>
                                                            <ObjectAnimationUsingKeyFrames Storyboard.TargetName="ContentPresenter" Storyboard.TargetProperty="Foreground">
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource AccentButtonForegroundPointerOver}" />
                                                            </ObjectAnimationUsingKeyFrames>
                                                        </Storyboard>
                                                    </VisualState>)" LR"(
                                                    
                                                    <VisualState x:Name="Pressed">
                                                        <Storyboard>
                                                            <ObjectAnimationUsingKeyFrames Storyboard.TargetName="ContentPresenter" Storyboard.TargetProperty="Background">
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource AccentButtonBackgroundPressed}" />
                                                            </ObjectAnimationUsingKeyFrames>
                                                            <ObjectAnimationUsingKeyFrames Storyboard.TargetName="ContentPresenter" Storyboard.TargetProperty="BorderBrush">
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource AccentButtonBorderBrushPressed}" />
                                                            </ObjectAnimationUsingKeyFrames>
                                                            <ObjectAnimationUsingKeyFrames Storyboard.TargetName="ContentPresenter" Storyboard.TargetProperty="Foreground">
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource AccentButtonForegroundPressed}" />
                                                            </ObjectAnimationUsingKeyFrames>
                                                        </Storyboard>
                                                    </VisualState>
                                                    
                                                    <VisualState x:Name="Disabled">
                                                        <Storyboard>
                                                            <ObjectAnimationUsingKeyFrames Storyboard.TargetName="ContentPresenter" Storyboard.TargetProperty="Background">
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource AccentButtonBackgroundDisabled}" />
                                                            </ObjectAnimationUsingKeyFrames>
                                                            <ObjectAnimationUsingKeyFrames Storyboard.TargetName="ContentPresenter" Storyboard.TargetProperty="BorderBrush">
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource AccentButtonBorderBrushDisabled}" />
                                                            </ObjectAnimationUsingKeyFrames>
                                                            <ObjectAnimationUsingKeyFrames Storyboard.TargetName="ContentPresenter" Storyboard.TargetProperty="Foreground">
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource AccentButtonForegroundDisabled}" />
                                                            </ObjectAnimationUsingKeyFrames>
                                                        </Storyboard>
                                                    </VisualState>
                                                </VisualStateGroup>
                                            </VisualStateManager.VisualStateGroups>
                                        </ContentPresenter>
                                    </ControlTemplate>
                                </Setter.Value>
                            </Setter>
                        </Style>
                    </Button.Style>
                </Button>
            </Grid>
        </Grid>
    </Border>
</Grid>)").as<Grid>();
        }

        int argc = 0;
        std::unique_ptr<PWSTR[]> argv{ CommandLineToArgvW(GetCommandLine(), &argc) };
        for (int index = 1; index < argc; index++)
        {
            std::wstring fullArgument{ argv[index] };

            if (index == 1)
            {
                grid.FindName(L"AppTitle").as<TextBlock>().Text(fullArgument);
            }
            else if (index == 2)
            {
                // 20% Remaining Battery
                grid.FindName(L"Title").as<ContentControl>().Content(box_value(fullArgument));
            }
            else if (index == 3)
            {
                // In order to maintain battery life, please charge your device now, before it completely runs out of power!
                grid.FindName(L"Content").as<TextBlock>().Text(fullArgument);
            }
        }
        argv.reset();

        grid.FindName(L"CloseButton").as<Button>().Click([](auto&&, auto&&)
        {
            if (DestroyWindow(_hWnd) == FALSE)
            {
                if (desktopXamlSource)
                {
                    desktopXamlSource.Close();
                    desktopXamlSource = nullptr;
                }
                PostQuitMessage(0);
            }
        });
        desktopXamlSource.Content(grid);
    }

    ShowWindow(_hWnd, SW_SHOW);
    UpdateWindow(_hWnd);

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszClassName  = szWindowClass;

    return RegisterClassExW(&wcex);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_SIZE:
        {
            if (desktopXamlSource != nullptr)
            {
                auto interop = desktopXamlSource.as<IDesktopWindowXamlSourceNative>();
                HWND xamlHostHwnd = NULL;
                check_hresult(interop->get_WindowHandle(&xamlHostHwnd));
                RECT windowRect;
                ::GetClientRect(hWnd, &windowRect);
                ::SetWindowPos(xamlHostHwnd, NULL, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_SHOWWINDOW);
            }

            return 0;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
        case WM_DESTROY:
            if (desktopXamlSource)
            {
                desktopXamlSource.Close();
                desktopXamlSource = nullptr;
            }
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
