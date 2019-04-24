import {Component, OnInit} from '@angular/core';
import {ClientService} from './client/client.service';
import {RadioResponse} from './client/radio-response';
import {of, timer} from 'rxjs';
import {catchError, switchMap} from 'rxjs/internal/operators';
import {TranslateService} from '@ngx-translate/core';
import {HttpErrorResponse} from '@angular/common/http';


@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})
export class AppComponent implements OnInit {

  error: string = null;

  state: RadioResponse = new RadioResponse();

  constructor(private translate: TranslateService, private client: ClientService) {
    translate.setDefaultLang('en');
    const lang = translate.getBrowserLang();
    translate.use(lang ? lang : 'en');
  }

  ngOnInit(): void {
    // Update state every seconds
    timer(0, 1000).pipe(
      switchMap(
        () => this.client.getState().pipe(
          catchError(err => {
            let msg = 'Connection error';
            if(err instanceof HttpErrorResponse) {
              msg = 'Error ' + err.status + ' ' + err.statusText;
            }
            return of(msg);
          })
        )
      )
    ).subscribe(
      result => {
        if(typeof result === 'string') {
          this.error = result;
        }
        else {
          this.error = null;
          this.state = result;
        }
      },
      error => {}
    );

  }






}
