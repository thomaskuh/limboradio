import {Component, Input} from '@angular/core';
import {RadioResponse} from '../client/radio-response';
import {ClientService} from '../client/client.service';
import {RadioRequest} from '../client/radio-request';

@Component({
  selector: 'app-card-config',
  templateUrl: './card-config.component.html',
  styleUrls: ['./card-config.component.scss']
})
export class CardConfigComponent {

  @Input()
  state: RadioResponse = new RadioResponse();

  constructor(private client: ClientService) { }

  setMode(e: any) {
    const req = new RadioRequest();
    req.mode = this.state.mode;
    this.client.saveThatThang(req).subscribe();
  }

  setThreshold(e: any) {
    const req = new RadioRequest();
    req.threshold = this.state.threshold;
    this.client.saveThatThang(req).subscribe();
  }

  setTimeout(e: any) {
    const req = new RadioRequest();
    req.timeout = this.state.timeout;
    this.client.saveThatThang(req).subscribe();
  }


}
