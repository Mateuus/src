#pragma once


namespace ParticleAtlas {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			cbGenerateMips->Enabled = false;
			btnCompose->Enabled = false;
			slicesX->Enabled = false;
			slicesY->Enabled = false;
			btnDecompose->Enabled = false;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  btnCompose;
	private: System::Windows::Forms::Button^  btnDecompose;
	protected: 

	protected: 

	private: System::Windows::Forms::FolderBrowserDialog^  selectFolder;
	private: System::Windows::Forms::OpenFileDialog^  selectFile;
	private: System::Windows::Forms::TextBox^  textureName;



	private: System::Windows::Forms::Button^  button3;
	private: System::Windows::Forms::NumericUpDown^  slicesY;

	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::NumericUpDown^  slicesX;

	private: System::Windows::Forms::Button^  button4;
	private: System::Windows::Forms::TextBox^  folderName;
	private: System::Windows::Forms::CheckBox^  cbGenerateMips;


	private: System::Windows::Forms::GroupBox^  groupBox1;
	private: System::Windows::Forms::GroupBox^  groupBox2;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->btnCompose = (gcnew System::Windows::Forms::Button());
			this->btnDecompose = (gcnew System::Windows::Forms::Button());
			this->selectFolder = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->selectFile = (gcnew System::Windows::Forms::OpenFileDialog());
			this->textureName = (gcnew System::Windows::Forms::TextBox());
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->slicesY = (gcnew System::Windows::Forms::NumericUpDown());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->slicesX = (gcnew System::Windows::Forms::NumericUpDown());
			this->button4 = (gcnew System::Windows::Forms::Button());
			this->folderName = (gcnew System::Windows::Forms::TextBox());
			this->cbGenerateMips = (gcnew System::Windows::Forms::CheckBox());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->slicesY))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->slicesX))->BeginInit();
			this->groupBox1->SuspendLayout();
			this->groupBox2->SuspendLayout();
			this->SuspendLayout();
			// 
			// btnCompose
			// 
			this->btnCompose->Location = System::Drawing::Point(4, 48);
			this->btnCompose->Name = L"btnCompose";
			this->btnCompose->Size = System::Drawing::Size(105, 27);
			this->btnCompose->TabIndex = 0;
			this->btnCompose->Text = L"Compose";
			this->btnCompose->UseVisualStyleBackColor = true;
			this->btnCompose->Click += gcnew System::EventHandler(this, &Form1::btnCompose_Click);
			// 
			// btnDecompose
			// 
			this->btnDecompose->Location = System::Drawing::Point(4, 49);
			this->btnDecompose->Name = L"btnDecompose";
			this->btnDecompose->Size = System::Drawing::Size(105, 29);
			this->btnDecompose->TabIndex = 1;
			this->btnDecompose->Text = L"Decompose";
			this->btnDecompose->UseVisualStyleBackColor = true;
			this->btnDecompose->Click += gcnew System::EventHandler(this, &Form1::btnDecompose_Click);
			// 
			// selectFile
			// 
			this->selectFile->FileName = L"selectFile";
			this->selectFile->Filter = L"Targa|*.tga|Photoshop|*.psd|Bitmap|*.bmp|JPEG|*.jpg|DDS|*.dds";
			// 
			// textureName
			// 
			this->textureName->Location = System::Drawing::Point(4, 17);
			this->textureName->Margin = System::Windows::Forms::Padding(1);
			this->textureName->Name = L"textureName";
			this->textureName->ReadOnly = true;
			this->textureName->Size = System::Drawing::Size(184, 20);
			this->textureName->TabIndex = 2;
			// 
			// button3
			// 
			this->button3->Location = System::Drawing::Point(190, 16);
			this->button3->Margin = System::Windows::Forms::Padding(1);
			this->button3->Name = L"button3";
			this->button3->Size = System::Drawing::Size(24, 20);
			this->button3->TabIndex = 3;
			this->button3->Text = L"...";
			this->button3->UseVisualStyleBackColor = true;
			this->button3->Click += gcnew System::EventHandler(this, &Form1::button3_Click);
			// 
			// slicesY
			// 
			this->slicesY->Location = System::Drawing::Point(310, 17);
			this->slicesY->Margin = System::Windows::Forms::Padding(1, 3, 3, 3);
			this->slicesY->Name = L"slicesY";
			this->slicesY->Size = System::Drawing::Size(46, 20);
			this->slicesY->TabIndex = 4;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(294, 23);
			this->label1->Margin = System::Windows::Forms::Padding(3, 0, 0, 0);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(15, 13);
			this->label1->TabIndex = 6;
			this->label1->Text = L"y:";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(223, 25);
			this->label2->Margin = System::Windows::Forms::Padding(3, 0, 0, 0);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(15, 13);
			this->label2->TabIndex = 8;
			this->label2->Text = L"x:";
			// 
			// slicesX
			// 
			this->slicesX->Location = System::Drawing::Point(239, 18);
			this->slicesX->Margin = System::Windows::Forms::Padding(1, 3, 3, 3);
			this->slicesX->Name = L"slicesX";
			this->slicesX->Size = System::Drawing::Size(46, 20);
			this->slicesX->TabIndex = 7;
			// 
			// button4
			// 
			this->button4->Location = System::Drawing::Point(190, 17);
			this->button4->Margin = System::Windows::Forms::Padding(1);
			this->button4->Name = L"button4";
			this->button4->Size = System::Drawing::Size(24, 20);
			this->button4->TabIndex = 10;
			this->button4->Text = L"...";
			this->button4->UseVisualStyleBackColor = true;
			this->button4->Click += gcnew System::EventHandler(this, &Form1::button4_Click);
			// 
			// folderName
			// 
			this->folderName->Location = System::Drawing::Point(4, 17);
			this->folderName->Margin = System::Windows::Forms::Padding(1);
			this->folderName->Name = L"folderName";
			this->folderName->ReadOnly = true;
			this->folderName->Size = System::Drawing::Size(184, 20);
			this->folderName->TabIndex = 9;
			// 
			// cbGenerateMips
			// 
			this->cbGenerateMips->AutoSize = true;
			this->cbGenerateMips->Location = System::Drawing::Point(228, 19);
			this->cbGenerateMips->Name = L"cbGenerateMips";
			this->cbGenerateMips->Size = System::Drawing::Size(95, 17);
			this->cbGenerateMips->TabIndex = 11;
			this->cbGenerateMips->Text = L"Generate Mips";
			this->cbGenerateMips->UseVisualStyleBackColor = true;
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->textureName);
			this->groupBox1->Controls->Add(this->btnDecompose);
			this->groupBox1->Controls->Add(this->button3);
			this->groupBox1->Controls->Add(this->slicesY);
			this->groupBox1->Controls->Add(this->label2);
			this->groupBox1->Controls->Add(this->label1);
			this->groupBox1->Controls->Add(this->slicesX);
			this->groupBox1->Location = System::Drawing::Point(10, 121);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(375, 84);
			this->groupBox1->TabIndex = 12;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Atlas Decomposition";
			// 
			// groupBox2
			// 
			this->groupBox2->Controls->Add(this->folderName);
			this->groupBox2->Controls->Add(this->btnCompose);
			this->groupBox2->Controls->Add(this->cbGenerateMips);
			this->groupBox2->Controls->Add(this->button4);
			this->groupBox2->Location = System::Drawing::Point(10, 12);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(375, 81);
			this->groupBox2->TabIndex = 13;
			this->groupBox2->TabStop = false;
			this->groupBox2->Text = L"Atlas Composition";
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(418, 231);
			this->Controls->Add(this->groupBox2);
			this->Controls->Add(this->groupBox1);
			this->Name = L"Form1";
			this->Text = L"Particle Atlas";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->slicesY))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->slicesX))->EndInit();
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			this->groupBox2->ResumeLayout(false);
			this->groupBox2->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void button4_Click(System::Object^  sender, System::EventArgs^  e) 
			 {
				 System::Windows::Forms::DialogResult res = selectFolder->ShowDialog();
				 folderName->Text = (res == System::Windows::Forms::DialogResult::OK) ? selectFolder->SelectedPath : L"";
				 cbGenerateMips->Enabled = folderName->Text->Length > 0;
				 btnCompose->Enabled = cbGenerateMips->Enabled;
			 }

	private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) 
			 {
				 System::Windows::Forms::DialogResult res = selectFile->ShowDialog();
				 textureName->Text = (res == System::Windows::Forms::DialogResult::OK) ? selectFile->FileName : L"";
				 slicesX->Enabled = textureName->Text->Length > 0;
				 slicesY->Enabled = slicesX->Enabled;
				 btnDecompose->Enabled = slicesX->Enabled;
			 }
	private: System::Void btnCompose_Click(System::Object^  sender, System::EventArgs^  e) 
			 {
				System::String^ cmd = folderName->Text;
				if(cbGenerateMips->Checked)
					cmd += " m";
				
				 System::Diagnostics::Process::Start("AtlasComposer.Release.exe", cmd);
			 }
	private: System::Void btnDecompose_Click(System::Object^  sender, System::EventArgs^  e) 
			 {
				 if(textureName->Text->Length > 0)
				 {
					 System::String^ cmd = textureName->Text + L" " + slicesX->Value.ToString() + L" " + slicesY->Value.ToString();
					 System::Diagnostics::Process::Start("AtlasDecomposer.Release.exe", cmd);
				 }			 
			 }
	};
}


