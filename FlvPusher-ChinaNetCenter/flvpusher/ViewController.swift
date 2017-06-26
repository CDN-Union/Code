//
//  ViewController.swift
//  flvpusher
//
//  Created by zous on 2017/2/11.
//  Copyright © 2017年 zous. All rights reserved.
//

import UIKit
import Foundation
import Alamofire

class ViewController: UIViewController {

    let BASE_URL: String = ""
    
    @IBOutlet weak var txtRandomId: UITextField!
    @IBOutlet weak var txtBaseUrl: UITextField!
    @IBOutlet weak var btnStart: UIButton!
    @IBOutlet weak var btnStop: UIButton!
    @IBOutlet weak var txtViewLog: UITextView!
    
    var threadPusher: Thread!
    var baseUrl: String = ""
    var randomId: String = ""
    var isRunning: Bool = false
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        initView();
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    @IBAction func onClickBtnStart(_ sender: UIButton) {
        randomId = txtRandomId!.text!
        baseUrl = txtBaseUrl!.text!
        
        UserDefaults.standard.set(randomId, forKey: "randomId")
        UserDefaults.standard.set(baseUrl, forKey: "baseUrl")
        
        threadPusher = Thread(target: self, selector: #selector(pusherMain), object: nil)
        threadPusher.start()
        btnStart.isEnabled = false
        btnStop.isEnabled = true
    }

    @IBAction func onClickBtnStop(_ sender: UIButton) {
        showPushLog(log: "正在停止推流...")
        isRunning = false
        btnStart.isEnabled = true
        btnStop.isEnabled = false
        stopPushStream()
    }
    
    func initView() {
        var rid = UserDefaults.standard.string(forKey: "randomId")
        if rid == nil {
            rid = String(arc4random_uniform(90000) + 10000)
        }
        var url = UserDefaults.standard.string(forKey: "baseUrl")
        if url == nil {
            url = BASE_URL
        }
        btnStart.isEnabled = true
        btnStop.isEnabled = false
        txtBaseUrl.text = url
        txtRandomId.text = rid
        txtViewLog.isEditable = false
        txtViewLog.isSelectable = false
        txtViewLog.text = ""
        showPushLog(log: "推流未开始.")
    }
    
    func showPushLog(log: String) {
        performSelector(onMainThread: #selector(showPushLogImpl), with: log, waitUntilDone: true)
    }
    
    func showPushLogImpl(log: String) {
        print("add log\(log)")
        let date = Date()
        let df = DateFormatter()
        df.setLocalizedDateFormatFromTemplate("HH:mm:ss")
        let fullLog = "\(txtViewLog.text!)[\(df.string(from: date))]\(log)\n"
        txtViewLog.text = fullLog
        let range = NSMakeRange(txtViewLog.text.characters.count - 1, 1)
        txtViewLog.scrollRangeToVisible(range)
    }
    
    func postLog(log: String, useWmp: Bool) {
        
    }
    
    func pusherMain() {
        isRunning = true
        var useWmp = false
        var round = 1
        showPushLog(log: "推流开始.")
        
        let logPath = NSTemporaryDirectory() + "report.log"
        let flvPath = Bundle.main.path(forResource: "ws_cut", ofType: "flv")!
        //showPushLog(log: "flv文件路径:\(flvPath)")
        while(isRunning) {
            let url = "\(baseUrl)"
            showPushLog(log: "第\(round)轮推流，地址：\(url) ")
            showPushLog(log: "推流中...")
            let cUrl  = url.cString(using: String.Encoding.ascii)
            let cFlvPath = flvPath.cString(using: String.Encoding.ascii)
            let cLogPath = logPath.cString(using: String.Encoding.ascii)
            startPushStream(cUrl, cFlvPath, cLogPath, useWmp ? 1 : 0)
            showPushLog(log: "第\(round)轮推流结束")
            postLog(log: logPath, useWmp: useWmp)
            useWmp = !useWmp
            round += 1
            sleep(2)
        }
        showPushLog(log: "推流已停止.")
    }
}

public extension UIDevice {
    
    var modelName: String {
        var systemInfo = utsname()
        uname(&systemInfo)
        let machineMirror = Mirror(reflecting: systemInfo.machine)
        let identifier = machineMirror.children.reduce("") { identifier, element in
            guard let value = element.value as? Int8, value != 0 else { return identifier }
            return identifier + String(UnicodeScalar(UInt8(value)))
        }
        
        switch identifier {
        case "iPod5,1":                                 return "iPod Touch 5"
        case "iPod7,1":                                 return "iPod Touch 6"
        case "iPhone3,1", "iPhone3,2", "iPhone3,3":     return "iPhone 4"
        case "iPhone4,1":                               return "iPhone 4s"
        case "iPhone5,1", "iPhone5,2":                  return "iPhone 5"
        case "iPhone5,3", "iPhone5,4":                  return "iPhone 5c"
        case "iPhone6,1", "iPhone6,2":                  return "iPhone 5s"
        case "iPhone7,2":                               return "iPhone 6"
        case "iPhone7,1":                               return "iPhone 6 Plus"
        case "iPhone8,1":                               return "iPhone 6s"
        case "iPhone8,2":                               return "iPhone 6s Plus"
        case "iPhone8,4":                               return "iPhone SE"
        case "iPhone9,1":                               return "iPhone 7"
        case "iPhone9,2":                               return "iPhone 7 Plus"
        case "iPad2,1", "iPad2,2", "iPad2,3", "iPad2,4":return "iPad 2"
        case "iPad3,1", "iPad3,2", "iPad3,3":           return "iPad 3"
        case "iPad3,4", "iPad3,5", "iPad3,6":           return "iPad 4"
        case "iPad4,1", "iPad4,2", "iPad4,3":           return "iPad Air"
        case "iPad5,3", "iPad5,4":                      return "iPad Air 2"
        case "iPad2,5", "iPad2,6", "iPad2,7":           return "iPad Mini"
        case "iPad4,4", "iPad4,5", "iPad4,6":           return "iPad Mini 2"
        case "iPad4,7", "iPad4,8", "iPad4,9":           return "iPad Mini 3"
        case "iPad5,1", "iPad5,2":                      return "iPad Mini 4"
        case "iPad6,7", "iPad6,8":                      return "iPad Pro"
        case "AppleTV5,3":                              return "Apple TV"
        case "i386", "x86_64":                          return "Simulator"
        default:                                        return identifier
        }
    }
}
