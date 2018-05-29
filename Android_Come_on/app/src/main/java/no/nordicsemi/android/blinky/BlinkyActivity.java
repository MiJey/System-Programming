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

package no.nordicsemi.android.blinky;

import android.arch.lifecycle.ViewModelProviders;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Switch;
import android.widget.TextView;

import no.nordicsemi.android.blinky.adapter.ExtendedBluetoothDevice;
import no.nordicsemi.android.blinky.viewmodels.BlinkyViewModel;

public class BlinkyActivity extends AppCompatActivity {
	public static final String EXTRA_DEVICE = "no.nordicsemi.android.blinky.EXTRA_DEVICE";

	@Override
	protected void onCreate(final Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_blinky);

		final Intent intent = getIntent();
		final ExtendedBluetoothDevice device = intent.getParcelableExtra(EXTRA_DEVICE);
		final String deviceName = device.getName();
		final String deviceAddress = device.getAddress();

		final Toolbar toolbar = findViewById(R.id.toolbar);
		setSupportActionBar(toolbar);
		getSupportActionBar().setTitle(deviceName);
		getSupportActionBar().setSubtitle(deviceAddress);
		getSupportActionBar().setDisplayHomeAsUpEnabled(true);

		// Configure the view model
		final BlinkyViewModel viewModel = ViewModelProviders.of(this).get(BlinkyViewModel.class);
		viewModel.connect(device);

		// Set up views
		final LinearLayout progressContainer = findViewById(R.id.progress_container);
		final TextView connectionState = findViewById(R.id.connection_state);
		final View content = findViewById(R.id.device_container);

		final Button p2_button0 = findViewById(R.id.p2_button0);
        final Button p2_button1 = findViewById(R.id.p2_button1);
        final Button p2_button2 = findViewById(R.id.p2_button2);
        final Button p2_button3 = findViewById(R.id.p2_button3);

        final TextView p1_button0 = findViewById(R.id.p1_button0);
        final TextView p1_button1 = findViewById(R.id.p1_button1);
        final TextView p1_button2 = findViewById(R.id.p1_button2);
        final TextView p1_button3 = findViewById(R.id.p1_button3);

		viewModel.isDeviceReady().observe(this, deviceReady -> {
			progressContainer.setVisibility(View.GONE);
			content.setVisibility(View.VISIBLE);
		});
		viewModel.getConnectionState().observe(this, connectionState::setText);
		viewModel.isConnected().observe(this, connected -> { if (!connected) { finish(); } });

		// 휴대폰(Player 2)에서 버튼 눌렀을 떄
        p2_button0.setOnClickListener(view -> viewModel.pressP2button(0));
        p2_button1.setOnClickListener(view -> viewModel.pressP2button(1));
        p2_button2.setOnClickListener(view -> viewModel.pressP2button(2));
        p2_button3.setOnClickListener(view -> viewModel.pressP2button(3));

		// 보드(Player 1)에서 버튼 눌렀을 때
        viewModel.getButtonState(0).observe(this, pressed ->
                p1_button0.setText("P1 Fn button0 " + (pressed ? "Pressed" : "Released")));
        viewModel.getButtonState(1).observe(this, pressed ->
                p1_button1.setText("P1 ↑ button1 " + (pressed ? "Pressed" : "Released")));
        viewModel.getButtonState(2).observe(this, pressed ->
                p1_button2.setText("P1 ← button2 " + (pressed ? "Pressed" : "Released")));
        viewModel.getButtonState(3).observe(this, pressed ->
                p1_button3.setText("P1 → button3 " + (pressed ? "Pressed" : "Released")));
	}

	@Override
	public boolean onOptionsItemSelected(final MenuItem item) {
		switch (item.getItemId()) {
			case android.R.id.home:
				onBackPressed();
				return true;
		}
		return false;
	}
}
