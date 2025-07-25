package com.example.app;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("app");  // ⚠️ Nombre correcto de la librería nativa
    }

    private DrawingView drawingView;
    private Spinner spinnerDescriptor;
    private TextView textResult;
    private TextView textSelectedDescriptor;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // 🔹 Inicializar AssetManager para código nativo
        initAssetManager(getAssets());

        // Referencias UI
        drawingView = findViewById(R.id.drawing_view);
        spinnerDescriptor = findViewById(R.id.spinner_descriptor);
        textResult = findViewById(R.id.text_result);
        textSelectedDescriptor = findViewById(R.id.text_selected_descriptor);
        Button buttonClear = findViewById(R.id.button_clear);
        Button buttonAnalyze = findViewById(R.id.button_analyze);

        // Configurar Spinner con descriptores
        String[] opciones = {"Momentos de Hu", "Shape Signature"};
        ArrayAdapter<String> adapter = new ArrayAdapter<>(
                this,
                android.R.layout.simple_spinner_item,
                opciones
        );
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinnerDescriptor.setAdapter(adapter);

        // Mostrar descriptor seleccionado dinámicamente
        spinnerDescriptor.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                String descriptorName = parent.getItemAtPosition(position).toString();
                textSelectedDescriptor.setText("Descriptor seleccionado: " + descriptorName);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                textSelectedDescriptor.setText("Descriptor seleccionado: ---");
            }
        });

        // Botón limpiar canvas
        buttonClear.setOnClickListener(v -> {
            drawingView.clear();
            textResult.setText("Resultado: ---");
        });

        // Botón analizar figura
        buttonAnalyze.setOnClickListener(v -> {
            Bitmap bitmap = drawingView.getBitmap();
            int descriptor = spinnerDescriptor.getSelectedItemPosition();  // 0: Hu, 1: SS

            String resultado = classifyShape(bitmap, descriptor);
            textResult.setText("Resultado: " + resultado);
        });
    }

    // Métodos nativos:
    public native void initAssetManager(AssetManager assetManager);
    public native String classifyShape(Bitmap bitmap, int descriptor);
}
