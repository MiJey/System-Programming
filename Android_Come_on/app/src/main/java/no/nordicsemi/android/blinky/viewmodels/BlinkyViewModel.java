/*
 * Copyright (c) 2015, Nordic Semiconductor
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *  Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 *  Neither the name of copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package no.nordicsemi.android.blinky.viewmodels;

import android.app.Application;
import android.arch.lifecycle.AndroidViewModel;
import android.arch.lifecycle.LiveData;
import android.arch.lifecycle.MutableLiveData;
import android.bluetooth.BluetoothDevice;
import android.support.annotation.NonNull;
import android.util.Log;

import no.nordicsemi.android.blinky.R;
import no.nordicsemi.android.blinky.adapter.ExtendedBluetoothDevice;
import no.nordicsemi.android.blinky.profile.BlinkyManager;
import no.nordicsemi.android.blinky.profile.BlinkyManagerCallbacks;
import no.nordicsemi.android.log.LogSession;
import no.nordicsemi.android.log.Logger;

public class BlinkyViewModel extends AndroidViewModel implements BlinkyManagerCallbacks {
	private final BlinkyManager mBlinkyManager;

	// Connection states Connecting, Connected, Disconnecting, Disconnected etc.
	private final MutableLiveData<String> mConnectionState = new MutableLiveData<>();

	// Flag to determine if the device is connected
	private final MutableLiveData<Boolean> mIsConnected = new MutableLiveData<>();

	// Flag to determine if the device is ready
	private final MutableLiveData<Void> mOnDeviceReady = new MutableLiveData<>();

	// Flag that holds the on off state of the LED. On is true, Off is False
	private final MutableLiveData<Boolean> mLEDState = new MutableLiveData<>();

	// Flag that holds the pressed released state of the button on the devkit. Pressed is true, Released is False
	private final MutableLiveData<Boolean> p1Button0State = new MutableLiveData<>();
	private final MutableLiveData<Boolean> p1Button1State = new MutableLiveData<>();
	private final MutableLiveData<Boolean> p1Button2State = new MutableLiveData<>();
	private final MutableLiveData<Boolean> p1Button3State = new MutableLiveData<>();

	public LiveData<Void> isDeviceReady() {
		return mOnDeviceReady;
	}

	public LiveData<String> getConnectionState() {
		return mConnectionState;
	}

	public LiveData<Boolean> isConnected() {
		return mIsConnected;
	}

//	public LiveData<Boolean> getButtonState() {
//		return p1Button0State;
//	}

	public LiveData<Boolean> getButtonState(int num) {
		switch (num){
			case 0: return p1Button0State;
			case 1: return p1Button1State;
			case 2: return p1Button2State;
			case 3: return p1Button3State;
			default: return p1Button3State;
		}
	}

	public LiveData<Boolean> getLEDState() {
		return mLEDState;
	}

	public BlinkyViewModel(@NonNull final Application application) {
		super(application);

		// Initialize the manager
		mBlinkyManager = new BlinkyManager(getApplication());
		mBlinkyManager.setGattCallbacks(this);
	}

	/**
	 * Connect to peripheral
	 */
	public void connect(final ExtendedBluetoothDevice device) {
		final LogSession logSession = Logger.newSession(getApplication(), null, device.getAddress(), device.getName());
		mBlinkyManager.setLogger(logSession);
		mBlinkyManager.connect(device.getDevice());
	}

	/**
	 * Disconnect from peripheral
	 */
	private void disconnect() {
		mBlinkyManager.disconnect();
	}

	// 휴대폰(Player 2)에서 버튼 눌렀을 떄 send
	public void pressP2button(final int p2button) {
		Log.d("buttonTest", "button: " + p2button);
		mBlinkyManager.send(p2button);
	}

	@Override
	protected void onCleared() {
		super.onCleared();
		if (mBlinkyManager.isConnected()) {
			disconnect();
		}
	}

	// 보드(Player 1)에서 버튼 눌렀을 떄 received
    @Override
    public void onDataReceived(int state) {
	    // pin_no가 26, 27, 28, 29
        switch(state){
            case 26: p1Button0State.postValue(false); break;
            case 27: p1Button0State.postValue(true); break;
            case 28: p1Button1State.postValue(false); break;
            case 29: p1Button1State.postValue(true); break;
            case 30: p1Button2State.postValue(false); break;
            case 31: p1Button2State.postValue(true); break;
            case 32: p1Button3State.postValue(false); break;
            case 33: p1Button3State.postValue(true); break;
        }
    }

    @Override
	public void onDataSent(final boolean state) {
		mLEDState.postValue(state);
	}

	@Override
	public void onDeviceConnecting(final BluetoothDevice device) {
		mConnectionState.postValue(getApplication().getString(R.string.state_connecting));
	}

	@Override
	public void onDeviceConnected(final BluetoothDevice device) {
		mIsConnected.postValue(true);
		mConnectionState.postValue(getApplication().getString(R.string.state_discovering_services));
	}

	@Override
	public void onDeviceDisconnecting(final BluetoothDevice device) {
		mIsConnected.postValue(false);
	}

	@Override
	public void onDeviceDisconnected(final BluetoothDevice device) {
		mIsConnected.postValue(false);
	}

	@Override
	public void onLinklossOccur(final BluetoothDevice device) {
		mIsConnected.postValue(false);
	}

	@Override
	public void onServicesDiscovered(final BluetoothDevice device, final boolean optionalServicesFound) {
		mConnectionState.postValue(getApplication().getString(R.string.state_initializing));
	}

	@Override
	public void onDeviceReady(final BluetoothDevice device) {
		mConnectionState.postValue(getApplication().getString(R.string.state_discovering_services_completed, device.getName()));
		mOnDeviceReady.postValue(null);
	}

	@Override
	public boolean shouldEnableBatteryLevelNotifications(final BluetoothDevice device) {
		// Blinky doesn't have Battery Service
		return false;
	}

	@Override
	public void onBatteryValueReceived(final BluetoothDevice device, final int value) {
		// Blinky doesn't have Battery Service
	}

	@Override
	public void onBondingRequired(final BluetoothDevice device) {
		// Blinky does not require bonding
	}

	@Override
	public void onBonded(final BluetoothDevice device) {
		// Blinky does not require bonding
	}

	@Override
	public void onError(final BluetoothDevice device, final String message, final int errorCode) {
		// TODO implement
	}

	@Override
	public void onDeviceNotSupported(final BluetoothDevice device) {
		// TODO implement
	}
}
