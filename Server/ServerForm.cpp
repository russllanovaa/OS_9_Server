#include "ServerForm.h"

static bool moderate = true;

System::Void Server::ServerForm::button1_Click(System::Object^ sender, System::EventArgs^ e)
{
    if (moderate == false)
    {
        label5->Text = "ON";
        label5->ForeColor = System::Drawing::Color::Green;
        moderate = true;
    }
    else
    {
        label5->Text = "OFF";
        label5->ForeColor = System::Drawing::Color::Red;
        moderate = false;
    }

}
}

System::Void Server::ServerForm::button2_Click(System::Object^ sender, System::EventArgs^ e)
{
    return System::Void();
}

System::Void Server::ServerForm::button3_Click(System::Object^ sender, System::EventArgs^ e)
{
    return System::Void();
}
