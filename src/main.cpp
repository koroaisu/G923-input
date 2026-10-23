#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <dinput.h>
#include <iostream>


// DirectInputが機器を見つけたときに呼ぶ関数。
// 最初に見つかった1台のIDを、mainのcontrollerIdに保存する。
BOOL CALLBACK enumController(
    const DIDEVICEINSTANCEW* device,// 見つかった機器の情報を指すポインター（読み取り専用）
    void* context// EnumDevicesから引き渡された保存先（&controllerId）
){
    GUID* destination = static_cast<GUID*>(context);// 保存先のアドレスをGUID*型に戻す。
    *destination = device->guidInstance;// 保存先の変数に、見つかった機器のIDをコピーする。

    return DIENUM_STOP;// 1台見つかったので、残りの探索を終了する。
}


int main() {
    // 機器の探索や、機器用オブジェクトの作成に使うポインター。
    IDirectInput8W* directInput = nullptr;

    // DirectInputを使うためのオブジェクトを作成する。
    HRESULT result = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8W, reinterpret_cast<void**>(&directInput), nullptr);
    
    if (FAILED(result)) {
        std::cerr << "DirectInput8インスタンスの作成が失敗しました。HRESULT: " << std::hex << result << std::endl;
        return 1;
    }

    GUID controllerId{};// 見つかった機器のIDの保存先。最初はゼロで初期化する。

    // 接続中のゲームコントローラーを探し、見つかるとenumControllerを呼ぶ。
    result = directInput->EnumDevices(
        DI8DEVCLASS_GAMECTRL,// ゲームコントローラーを対象にする。
        enumController,// 機器を見つけたときに呼ぶ関数。
        &controllerId,// コールバックに渡す、IDの保存先。
        DIEDFL_ATTACHEDONLY// 現在接続されている機器だけを対象にする。
    );
    if (FAILED(result)) {
        std::cerr << "列挙処理が失敗しました。HRESULT: " << std::hex << result << std::endl;
        directInput->Release();
        return 1;
    }
    // IDが初期値のままなら、機器が見つかっていない。
    if (IsEqualGUID(controllerId, GUID_NULL)) {
        std::cerr << "コントローラーが見つかりません" << std::endl;
        directInput->Release();
        return 1;
    }


    IDirectInputDevice8W* controller = nullptr;// 選んだ1台の設定や入力取得に使うポインター。

    
    // 保存したIDで機器を指定し、その機器用のオブジェクトを作成する。
    result = directInput->CreateDevice(controllerId, &controller, nullptr);
    if (FAILED(result)) {
        std::cerr << "コントローラーの作成に失敗しました。HRESULT: " << std::hex << result << std::endl;
        directInput->Release();
        return 1;
    }

    // 入力をDIJOYSTATE2形式で受け取るように設定する。
    result = controller->SetDataFormat(&c_dfDIJoystick2);
    if (FAILED(result)) {
        std::cerr << "入力の受け取り形式が指定に失敗しました。HRESULT: " << std::hex << result << std::endl;
        controller->Release();
        directInput->Release();
        return 1;
    }

    // この機器から入力を取得できる状態にする。
    result = controller->Acquire();
    if (FAILED(result)) {
        std::cerr << "コントローラーの取得に失敗しました。HRESULT: " << std::hex << result << std::endl;
        controller->Release();
        directInput->Release();
        return 1;
    }
    
    
    DIJOYSTATE2 state{};// 読み取った軸やボタンの状態の保存先。
    
    for (int i = 0; i < 100; ++i) {// 約5秒間、入力の読み取りと表示を繰り返す。
        // 機器に入力状態の更新を要求する。
        result = controller->Poll();
        if (FAILED(result)) {
            std::cerr << "コントローラーのポーリングに失敗しました。HRESULT: " << std::hex << result << std::endl;
            controller->Release();
            directInput->Release();
            return 1;
        }

        // 現在の入力状態をstateに書き込む。
        result = controller->GetDeviceState(sizeof(state), &state);
        if (FAILED(result)) {
            std::cerr << "コントローラーの状態取得に失敗しました。HRESULT: " << std::hex << result << std::endl;
            controller->Release();
            directInput->Release();
            return 1;
        }else {
            // このG923で操作して確認した、ハンドルと各ペダルの軸を表示する。
            std::cout << "X: " << state.lX // ハンドル：左端0 → 右端65535
            << " Y: " << state.lY// アクセル：離すと65535、踏み切ると0
            << " RZ: " << state.lRz// ブレーキ：離すと65535、踏み切ると0
            << " rglSlider[0]: " << state.rglSlider[0]// クラッチ：離すと65535、踏み切ると0

            << std::endl;
        }

        Sleep(50);// 次の読み取りまで50ミリ秒待つ。
    }
    

    // 入力取得の利用を終了する。
    controller->Unacquire();

    // 機器用オブジェクト、DirectInputの順に解放する。
    controller->Release();

    directInput->Release();

    return 0;
}
