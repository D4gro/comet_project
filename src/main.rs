use windows::{
    core::*, 
    Data::Xml::Dom::*,
    Win32::Foundation::*,
    Win32::System::Threading::*,
    Win32::UI::WindowsAndMessaging::*,
};

fn main() -> Result<()> {
    let doc = XmlDocument::new()?;
    doc.LoadXml(h!("<html>hi!</html>"))?;
    
    let root = doc.DocumentElement()?;
    assert!(root.NodeName()? == "html");
    assert!(root.InnerText()? == "hi!");

    unsafe {
        let event = CreateEventW(None, true, false, None)?;
        SetEvent(event)?;
        WaitForSingleObject(event, 0);
        CloseHandle(event)?;

        MessageBoxA(None, s!("Ansi"), s!("hi!A"), MB_OK);
        MessageBoxW(None, w!("Wide"), w!("hi!W"), MB_OK);
    }
    
    Ok(())
}